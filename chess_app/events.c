#include "Events.h"
#include "EventHandler.h"
#include "Overlay.h"

#include "../src/state/Board.h"
#include "../src/state/Move.h"
#include "../src/engine/MoveGenerator.h"
#include "../src/engine/ChessGameEmulator.h"
#include "../src/chessBot/ChessBot.h"
#include "../src/chessBot/RepetitionTable.h"

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
    u64 piecesBitBoard = Board_allPiecesBitBoard(gameState->currentState.board);
    int nbPieces = numBitSet_64(piecesBitBoard);
    if (nbPieces > 4) { return false; }

    u64 rooksPawnsQueens = piecesBitBoard & (
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(WHITE, QUEEN)) |
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(BLACK, QUEEN)) |
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(WHITE, ROOK)) |
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(BLACK, ROOK)) |
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(WHITE, PAWN)) |
                            Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(BLACK, PAWN))
                        );
    
    if (rooksPawnsQueens != (u64) 0) {
        // There is still pawns, rooks and/or queens on the board
        return false;
    }

    if (nbPieces <= 3) { 
        // Lone bishop or lone knight (nbPiece == 3) or two lone kings (nbPiece == 2)
        return true; 
    }

    int nbKWhiteKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(WHITE, KNIGHT)));
    int nbKBlackKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->currentState.board, Piece_makePiece(BLACK, KNIGHT)));
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
    if (gameState->currentState.colorToGo == WHITE) {
        if (gameState->blackRemainingTime <= (u32) 0) {
            gameState->result = WHITE_WON_ON_TIME;
            return;
        }
    } else {
        if (gameState->whiteRemainingTime <= (u32) 0) {
            gameState->result = BLACK_WON_ON_TIME;
            return;
        }
    }

    Move moves[256];
    int numMove;
    Engine_getValidMoves(moves, &numMove, gameState->currentState);
    if (numMove == 0) {
        if (Engine_isKingInCheck() || Engine_isKingInDoubleCheck()) {
            gameState->result = gameState->currentState.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        } else {
            gameState->result = STALEMATE;
        }
    } else if (RepetitionTable_isKeyContainedTwiceInTable(gameState->currentState.key)) {
        gameState->result = THREE_MOVE_REPETITION;
    } else if (gameState->currentState.turnsForFiftyRule > 50) {
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
    Engine_playMove(move, &gameState->currentState, true);
}

static inline void playBotMove(GameState *gameState) {
    Bot_provideGameStateForBot(&gameState->currentState);
    Move botMove = Bot_think(gameState->whiteRemainingTime, gameState->whiteRemainingTime);
    
    u64 currentTick = SDL_GetTicks64();
    if (gameState->playerColor != WHITE) {
        gameState->whiteRemainingTime -= (currentTick - gameState->turnStartTick);
    } else {
        gameState->blackRemainingTime -= (currentTick - gameState->turnStartTick);
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
    gameState->blackRemainingTime = STARTING_TIME_MS;
    gameState->whiteRemainingTime = STARTING_TIME_MS;
    // Note that we are not changing the player color
}

void clickedSwitchColorButton(SDL_Event event, AppState *appState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        appState->gameState.playerColor = appState->gameState.playerColor == WHITE ? BLACK : WHITE;
        resetGame(&appState->gameState);
        if (appState->gameState.currentState.colorToGo != appState->gameState.playerColor) {
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
        if (appState->gameState.currentState.colorToGo != appState->gameState.playerColor) {
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
        PieceCharacteristics currentColor = appState->gameState.currentState.colorToGo;
        switch (pieceIndex) {
            case 0: move = Move_makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_QUEEN); break;
            case 1: move = Move_makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_KNIGHT); break;
            case 2: move = Move_makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_ROOK); break;
            case 3: move = Move_makeMove(appState->draggingState.from, appState->draggingState.to, PROMOTE_TO_BISHOP); break;
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
    Engine_getValidMoves(moves, &numMoves, appState->gameState.currentState);

    for (int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        Move move = moves[moveIndex];

        if (Move_fromSquare(move) == appState->draggingState.from && Move_toSquare(move) == draggingTo) {
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square (except for promotion)
            if (Move_flag(move) == PROMOTE_TO_QUEEN || 
                Move_flag(move) == PROMOTE_TO_KNIGHT ||
                Move_flag(move) == PROMOTE_TO_ROOK ||
                Move_flag(move) == PROMOTE_TO_BISHOP) {
                
                appState->draggingState.to = draggingTo; // I set this to communicate it to the popup callback

                Popup popup = { 0 };
                popup.callback = &clickedPromotionOverlay;
                renderPromotionOverlay(appState->sdlState.renderer, 
                                       appState->textures, 
                                       appState->gameState.currentState.colorToGo, 
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

    if (Board_pieceAtIndex(gameState->currentState.board, square) == NOPIECE) { return; }

    draggingState->draggedPiece = Board_pieceAtIndex(gameState->currentState.board, square);
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
