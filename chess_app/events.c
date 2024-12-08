#include "Events.h"
#include "EventHandler.h"
#include "Overlay.h"

#include "../src/state/Board.h"
#include "../src/state/Move.h"
#include "../src/MoveGenerator.h"
#include "../src/ChessGameEmulator.h"
#include "../src/chessBot/ChessBot.h"

// Note: This will be correct if the point (x, y) is in the chessboard
inline static int squareFromxy(int x, int y, bool flip) {
    int squareSize = CHESSBOARD_WIDTH / BOARD_LENGTH;
    int col = (x - CHESSBOARD_X) / squareSize;
    int row = y / squareSize;
    if (flip) {
        col = BOARD_LENGTH - 1 - col;
        row = BOARD_LENGTH - 1 - row;
    }
    return row * BOARD_LENGTH + col;
}

/*
All insufficient material scenario are:
    - Two lone king
    - A lone knight and a lone bishop for each player
    - A lone bishop or a lone knight
    - Two knights for one side with a lone king for the opponent
        - Note: The last case sometimes has a forced mate, but not always
                and I don't want to deal with having to calculate if there is a possibility of a forced mate,
                so I'll say it's draw
*/
static inline bool insufficientMaterialScenario(const GameState *gameState) {
    u64 piecesBitBoard = allPiecesBitBoard(gameState->currentState.currentPosition.board);
    int nbPieces = numBitSet_64(piecesBitBoard);
    if (nbPieces > 4) { return false; }

    u64 rooksPawnsQueens = piecesBitBoard & (
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(WHITE, QUEEN)) |
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(BLACK, QUEEN)) |
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(WHITE, ROOK)) |
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(BLACK, ROOK)) |
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(WHITE, PAWN)) |
                            bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(BLACK, PAWN))
                        );
    
    if (rooksPawnsQueens != (u64) 0) {
        // There is still pawns, rooks and/or queens on the board
        return false;
    }

    if (nbPieces <= 3) { 
        // Lone bishop or lone knight (nbPiece == 3) or two lone kings (nbPiece == 2)
        return true; 
    }

    int nbKWhiteKnights = numBitSet_64(piecesBitBoard & bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(WHITE, KNIGHT)));
    int nbKBlackKnights = numBitSet_64(piecesBitBoard & bitBoardForPiece(gameState->currentState.currentPosition.board, makePiece(BLACK, KNIGHT)));
    if (nbKWhiteKnights != nbKBlackKnights) { 
        // Handles case of two knights on either side and lone knight and lone bishop
        return true; 
    }

    // The remaning scenarios are one bishop or knight for each player
    return false;
}

static void computeGameEnd(GameState *gameState) {
    // This checking of the currentState is pretty much only useful for 
    // the bot running out of time
    if (gameState->currentState.currentPosition.colorToGo == WHITE) {
        if (gameState->currentState.blackTimeMs <= (u32) 0) {
            gameState->result = WHITE_WON_ON_TIME;
            return;
        }
    } else {
        if (gameState->currentState.whiteTimeMs <= (u32) 0) {
            gameState->result = BLACK_WON_ON_TIME;
            return;
        }
    }

    Move moves[256];
    int numMove;
    getValidMoves(moves, &numMove, gameState->currentState.currentPosition);
    if (numMove == 0) {
        if (isKingInCheck() || isKingInDoubleCheck()) {
            gameState->result = gameState->currentState.currentPosition.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        } else {
            gameState->result = STALEMATE;
        }
    } else if (isThereThreeFoldRepetition(&gameState->currentState)) {
        gameState->result = THREE_MOVE_REPETITION;
    } else if (gameState->currentState.currentPosition.turnsForFiftyRule > 50) {
        // TODO: Check this case out
        gameState->result = FIFTY_MOVE_RULE;
    } else if (insufficientMaterialScenario(gameState)) {
        gameState->result = INSUFFICIENT_MATERIAL;
    }
}

static inline void playMoveOnBoard(GameState *gameState, Move move) {
    if (gameState->previousStateIndex >= gameState->previousStateCapacity) {
        gameState->previousStates = realloc(gameState->previousStates, sizeof(GameState) * gameState->previousStateCapacity * 2);
        gameState->previousStateCapacity *= 2;
    }
    gameState->previousStates[gameState->previousStateIndex++] = gameState->currentState;
    playMove(move, &gameState->currentState);
}

static inline void playBotMove(GameState *gameState) {
    provideGameStateForBot(&gameState->currentState);
    Move botMove = think();
    
    u64 currentTick = SDL_GetTicks64();
    if (gameState->playerColor != WHITE) {
        gameState->currentState.whiteTimeMs -= (currentTick - gameState->turnStartTick);
    } else {
        gameState->currentState.blackTimeMs -= (currentTick - gameState->turnStartTick);
    }
    gameState->turnStartTick = currentTick;

    if (botMove == BOT_ERROR) {
        // The bot thinks the game is done
        if (gameState->result == GAME_IS_NOT_DONE) {
            // The app does not think the game is done
            printf("ERROR:  The bot cannot play a move because it thinks the game is done yet app does not label the game as done\n");
        }
        return;
    }
    playMoveOnBoard(gameState, botMove);
}

static inline void playTurn(GameState *gameState, Move playerMove) {
    playMoveOnBoard(gameState, playerMove);
    computeGameEnd(gameState);
    if (gameState->result != GAME_IS_NOT_DONE) { return; }

    playBotMove(gameState);
    computeGameEnd(gameState);
}

static void resetGame(GameState *gameState) {
    if (gameState->previousStateIndex == 0) {
        return; // Game is already reset
    }
    gameState->currentState = gameState->previousStates[0];
    gameState->previousStateIndex = 0;
    gameState->result = GAME_IS_NOT_DONE;
    gameState->turnStartTick = SDL_GetTicks64();
    gameState->currentState.blackTimeMs = TIME_CONTROL_MS;
    gameState->currentState.whiteTimeMs = TIME_CONTROL_MS;
    // Note that we are not changing the player color
}

void clickedSwitchColorButton(SDL_Event event, AppState *appState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        appState->gameState.playerColor = appState->gameState.playerColor == WHITE ? BLACK : WHITE;
        resetGame(&appState->gameState);
        if (appState->gameState.currentState.currentPosition.colorToGo != appState->gameState.playerColor) {
            playBotMove(&appState->gameState);
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
}


void clickedRestartButton(SDL_Event event, AppState *appState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        resetGame(&appState->gameState);
        if (appState->gameState.currentState.currentPosition.colorToGo != appState->gameState.playerColor) {
            playBotMove(&appState->gameState);
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
}

static bool clickedPromotionOverlay(SDL_Event event, SDL_Rect popupRect, AppState *appState) {
    int mouseX, mouseY;
    Move move = 0;
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        SDL_GetMouseState(&mouseX, &mouseY);
        int squareSize = (WINDOW_WIDTH * 2 / 3) / BOARD_LENGTH;
        int relativeX = mouseX - popupRect.x;
        int relativeY = mouseY - popupRect.y;

        int colIndex = relativeX / squareSize; // 0 or 1
        int rowIndex = relativeY / squareSize; // 0 or 1

        // Map the row and column to a piece
        int pieceIndex = rowIndex * 2 + colIndex;
        PieceCharacteristics currentColor = appState->gameState.currentState.currentPosition.colorToGo;
        switch (pieceIndex) {
            case 0: move = makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_QUEEN); break;
            case 1: move = makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_KNIGHT); break;
            case 2: move = makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_ROOK); break;
            case 3: move = makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_BISHOP); break;
            default: break;
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
    if (move == 0) { return false; }

    playTurn(&appState->gameState, move);

    return true;
}

static void chessBoardMouseButtonUp(AppState *appState) {
    if (appState->gameState.result != GAME_IS_NOT_DONE || !appState->draggingState.isDragging) { return; } // We are not dragging anything or the game is done
    appState->draggingState.isDragging = false; // Always stop dragging when we stop holding click

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    int draggingTo = squareFromxy(mouseX, mouseY, appState->gameState.playerColor == BLACK);

    // Finding the valid moves of this position
    // We could cache this value if it really is that slow, but I don't think so
    // Here I used 256 because it is the closest power of 2 from MAX_LEGAL_MOVES
    Move moves[256];
    int numMoves;
    getValidMoves(moves, &numMoves, appState->gameState.currentState.currentPosition);

    for (int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        Move move = moves[moveIndex];

        if (fromSquare(move) == appState->draggingState.from && toSquare(move) == draggingTo) {
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square (except for promotion)
            if (flagFromMove(move) == PROMOTE_TO_QUEEN || 
                flagFromMove(move) == PROMOTE_TO_KNIGHT ||
                flagFromMove(move) == PROMOTE_TO_ROOK ||
                flagFromMove(move) == PROMOTE_TO_BISHOP) {
                
                appState->draggingState.to = draggingTo; // I set this to communicate it to the popup callback

                Popup popup = { 0 };
                popup.callback = &clickedPromotionOverlay;
                // TODO: See if this thing renders
                renderPromotionOverlay(appState->sdlState.renderer, 
                                       appState->textures, 
                                       appState->gameState.currentState.currentPosition.colorToGo, 
                                       draggingTo, 
                                       appState->gameState.playerColor,
                                       &popup);
                handlePopup(&popup, appState);
                return; // The callback will handle playing the turn
            }

            playTurn(&appState->gameState, move);
            break;
        }
    }
}

static void chessBoardMouseButtonDown(GameState *gameState, DraggingState *draggingState) {
    if (gameState->result != GAME_IS_NOT_DONE) { return; } // Game is done

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    int square = squareFromxy(mouseX, mouseY, gameState->playerColor == BLACK);

    if (pieceAtIndex(gameState->currentState.currentPosition.board, square) == NOPIECE) { return; }

    draggingState->draggedPiece = pieceAtIndex(gameState->currentState.currentPosition.board, square);
    draggingState->from = square;
    draggingState->isDragging = true;
}

void clickedChessBoard(SDL_Event event, AppState *appState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        chessBoardMouseButtonDown(&appState->gameState, &appState->draggingState);
        break;
    case SDL_MOUSEBUTTONUP:
        chessBoardMouseButtonUp(appState);
        break;
    default:
        break;
    }
}
