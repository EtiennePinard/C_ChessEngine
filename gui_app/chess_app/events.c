#include <stdio.h>

#include "../sdl_framework/EventHandler.h"

#include "../../engine/src/state/Board.h"
#include "../../engine/src/state/Move.h"
#include "../../engine/src/moveHandler/MoveGenerator.h"
#include "../../engine/src/moveHandler/MovePlayer.h"
#include "../../engine/src/bot/RepetitionTable.h"
#include "../../engine/src/utils/Math.h"
#include "../../engine/src/utils/FenString.h"

#include "uciEngineCommunication/UCIEngineCommunication.h"
#include "Overlay.h"
#include "Events.h"

// Note: This will be correct if the point (x, y) is in the chessboard
inline static Square squareFromxy(int x, int y, bool flip) {
    int squareSize = CHESSBOARD_WIDTH / BOARD_LENGTH;
    int col = (x - CHESSBOARD_X) / squareSize;
    int row = y / squareSize;
    if (flip) {
        col = BOARD_LENGTH - 1 - col;
        row = BOARD_LENGTH - 1 - row;
    }
    return (Square)row * BOARD_LENGTH + col;
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
static inline bool insufficientMaterialScenario(const GameState* gameState) {
    u64 piecesBitBoard = Board_allPiecesBitBoard(gameState->currentPosition.board);
    int nbPieces = numBitSet_64(piecesBitBoard);
    if (nbPieces > 4) { return false; }

    u64 rooksPawnsQueens = piecesBitBoard & (
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(WHITE, QUEEN)) |
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(BLACK, QUEEN)) |
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(WHITE, ROOK)) |
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(BLACK, ROOK)) |
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(WHITE, PAWN)) |
        Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(BLACK, PAWN))
        );

    if (rooksPawnsQueens != (u64)0) {
        // There is still pawns, rooks and/or queens on the board
        return false;
    }

    if (nbPieces <= 3) {
        // Lone bishop or lone knight (nbPiece == 3) or two lone kings (nbPiece == 2)
        return true;
    }

    int nbKWhiteKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(WHITE, KNIGHT)));
    int nbKBlackKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->currentPosition.board, Piece_makePiece(BLACK, KNIGHT)));
    if (nbKWhiteKnights != nbKBlackKnights) {
        // Handles case of two knights on either side and lone knight and lone bishop
        return true;
    }

    // The remaining scenarios are one bishop or knight for each player
    return false;
}

static void computeGameEnd(GameState* gameState) {
    // This checking of the currentState is pretty much only useful for 
    // the bot running out of time
    if (gameState->currentPosition.colorToGo == WHITE) {
        if (gameState->blackRemainingTime <= (u32)0) {
            gameState->result = WHITE_WON_ON_TIME;
            return;
        }
    }
    else {
        if (gameState->whiteRemainingTime <= (u32)0) {
            gameState->result = BLACK_WON_ON_TIME;
            return;
        }
    }

    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMove;
    MoveHandler_getValidMoves(moves, &numMove, gameState->currentPosition);
    if (numMove == 0) {
        if (MoveHandler_isKingInCheck() || MoveHandler_isKingInDoubleCheck()) {
            gameState->result = gameState->currentPosition.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        }
        else {
            gameState->result = STALEMATE;
        }
    }
    else if (RepetitionTable_isKeyContainedTwiceInTable(gameState->currentPosition.key)) {
        gameState->result = THREE_MOVE_REPETITION;
    }
    else if (gameState->currentPosition.turnsForFiftyRule > 50) {
        // TODO: Check this case out
        gameState->result = FIFTY_MOVE_RULE;
    }
    else if (insufficientMaterialScenario(gameState)) {
        gameState->result = INSUFFICIENT_MATERIAL;
    }
}

static inline void playMoveOnBoard(GameState* gameState, Move move) {
    if (gameState->undoStates.previousStateIndex >= gameState->undoStates.previousStateCapacity) {
        gameState->undoStates.previousStateCapacity *= 2;
        gameState->undoStates.previousStates = realloc(gameState->undoStates.previousStates, sizeof(GameState) * gameState->undoStates.previousStateCapacity);
        gameState->movesPlayed = realloc(gameState->movesPlayed, sizeof(Move) * gameState->undoStates.previousStateCapacity);
    }
    gameState->undoStates.previousStates[gameState->undoStates.previousStateIndex] = gameState->currentPosition;
    gameState->movesPlayed[gameState->undoStates.previousStateIndex] = move;
    gameState->undoStates.previousStateIndex++;

    MoveHandler_playMove(move, &gameState->currentPosition, true);
    computeGameEnd(gameState);
}

static int botMove(void* data) {
    GameState* gameState = (GameState*)data;
    ChessPosition startingPosition = (gameState->undoStates.previousStateIndex == 0) ?
        gameState->currentPosition :
        gameState->undoStates.previousStates[0];
    Move botMove = UCIEngine_bestMoveTimed(
        startingPosition,
        gameState->movesPlayed,
        gameState->undoStates.previousStateIndex,
        200
        // gameState->whiteRemainingTime,
        // gameState->blackRemainingTime,
        // gameState->whiteIncrement,
        // gameState->blackIncrement,
        // -1 // We don't have movesToGo for now
    );

    botMove = MoveHandler_correctMoveFlag(gameState->currentPosition, botMove);

    if (Move_fromSquare(botMove) == Move_toSquare(botMove) && gameState->result == GAME_IS_NOT_DONE) {
        printf("ERROR: The engine gave back a NULL_MOVE and the game is not done\n");
        exit(EXIT_FAILURE);
        return 1;
    }

    u64 currentTick = SDL_GetTicks64();
    if (gameState->playerColor != WHITE) gameState->whiteRemainingTime -= (currentTick - gameState->turnStartTick);
    else gameState->blackRemainingTime -= (currentTick - gameState->turnStartTick);
    gameState->turnStartTick = currentTick;
    playMoveOnBoard(gameState, botMove);

    return 0;
}

/**
 * @brief Plays a move chosen by the bot. This function takes
 * a long time and so it creates a thread to compute the bot
 * move. The pointer to this thread is returned, which means
 * it is the caller's responsibility to either wait for the
 * thread or detach it, depending if it needs to use the value
 * of the bot's move immediately. Use SDL_WaitThread(thread, NULL)
 * to wait for the thread or SDL_DetachThread(thread) to detach the
 * thread.
 *
 * IMPORTANT: The thread will modify the GameState, which means
 * that if you don't wait for the thread you cannot modify
 * the gameState will this thread has not finished since
 * it would create race conditions. To check if this
 * thread as finished from the gameState you can check if
 * its the player color to go. If it is the case, then
 * this function has finished executing.
 *
 * @param gameState The state of the game
 * @return SDL_Thread* The bot's thread
 */
static SDL_Thread* playBotMove(GameState* gameState) {
    SDL_Thread* thread = SDL_CreateThread(botMove, "botMove", gameState);
    if (thread == NULL) {
        fprintf(stderr, "Failed to create thread: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return thread;
}

static inline void playTurn(GameState* gameState, Move playerMove) {
    playMoveOnBoard(gameState, playerMove);
    if (gameState->result != GAME_IS_NOT_DONE) return;

    SDL_Thread* thread = playBotMove(gameState);
    SDL_DetachThread(thread);
}

static void resetGame(GameState* gameState) {
    // If we are not already at the beginning go back to the beginning
    ChessPosition startingPosition = (gameState->undoStates.previousStateIndex == 0) ?
        gameState->currentPosition :
        gameState->undoStates.previousStates[0];
    memcpy(&gameState->currentPosition, &startingPosition, sizeof(startingPosition));
    gameState->undoStates.previousStateIndex = 0;
    gameState->result = GAME_IS_NOT_DONE;
    gameState->blackRemainingTime = STARTING_TIME_MS;
    gameState->whiteRemainingTime = STARTING_TIME_MS;
    gameState->turnStartTick = SDL_GetTicks64();

    // Resetting the engine's internal game
    // note: ucinewgame does not have a response
    UCIEngine_sendCommand("ucinewgame");

    RepetitionTable_clear();
}

void clickedSwitchColorButton(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        app.state->gameState.playerColor = app.state->gameState.playerColor == WHITE ? BLACK : WHITE;
        resetGame(&app.state->gameState);
        if (app.state->gameState.currentPosition.colorToGo != app.state->gameState.playerColor) {
            SDL_Thread* thread = playBotMove(&app.state->gameState);
            SDL_DetachThread(thread);
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
}

void clickedRestartButton(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        resetGame(&app.state->gameState);
        if (app.state->gameState.currentPosition.colorToGo != app.state->gameState.playerColor) {
            SDL_Thread* thread = playBotMove(&app.state->gameState);
            SDL_DetachThread(thread);
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
}

static bool clickedPromotionOverlay(SDL_Event event, SDL_Rect popupRect, App app) {
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
        // PieceCharacteristics currentColor = app.state->gameState.currentPosition.colorToGo;
        switch (pieceIndex) {
        case 0: move = Move_makeMove(app.state->draggingState.from, app.state->draggingState.to, PROMOTE_TO_QUEEN); break;
        case 1: move = Move_makeMove(app.state->draggingState.from, app.state->draggingState.to, PROMOTE_TO_KNIGHT); break;
        case 2: move = Move_makeMove(app.state->draggingState.from, app.state->draggingState.to, PROMOTE_TO_ROOK); break;
        case 3: move = Move_makeMove(app.state->draggingState.from, app.state->draggingState.to, PROMOTE_TO_BISHOP); break;
        default: break;
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
    if (move == 0) { return false; }

    playTurn(&app.state->gameState, move);
    return true;
}

static void chessBoardMouseButtonUp(App app) {
    if (app.state->gameState.result != GAME_IS_NOT_DONE || !app.state->draggingState.isDragging) { return; } // We are not dragging anything or the game is done
    app.state->draggingState.isDragging = false; // Always stop dragging when we stop holding click

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    Square draggingTo = squareFromxy(mouseX, mouseY, app.state->gameState.playerColor == BLACK);

    // Finding the valid moves of this position
    // We could cache this value if it really is that slow, but I don't think so
    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMoves;
    MoveHandler_getValidMoves(moves, &numMoves, app.state->gameState.currentPosition);

    for (int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        Move move = moves[moveIndex];

        if (Move_fromSquare(move) == app.state->draggingState.from && Move_toSquare(move) == draggingTo) {
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square (except for promotion)
            if (Move_flag(move) == PROMOTE_TO_QUEEN ||
                Move_flag(move) == PROMOTE_TO_KNIGHT ||
                Move_flag(move) == PROMOTE_TO_ROOK ||
                Move_flag(move) == PROMOTE_TO_BISHOP) {

                app.state->draggingState.to = draggingTo; // I set this to communicate it to the popup callback

                Popup popup = { 0 };
                popup.callback = &clickedPromotionOverlay;
                renderPromotionOverlay(app.state->sdlState.renderer,
                    app.state->textures,
                    app.state->gameState.currentPosition.colorToGo,
                    draggingTo,
                    app.state->gameState.playerColor == BLACK,
                    &popup);
                handlePopup(&popup, app);
                return; // The callback will handle playing the turn
            }

            playTurn(&app.state->gameState, move);
            break;
        }
    }
}

static void chessBoardMouseButtonDown(GameState* gameState, DraggingState* draggingState) {
    // Game is done or it is not players turn to go
    if (gameState->result != GAME_IS_NOT_DONE ||
        gameState->playerColor != gameState->currentPosition.colorToGo) {
        return;
    }

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    int square = squareFromxy(mouseX, mouseY, gameState->playerColor == BLACK);

    if (Board_pieceAtIndex(gameState->currentPosition.board, square) == NOPIECE) return;

    draggingState->draggedPiece = Board_pieceAtIndex(gameState->currentPosition.board, square);
    draggingState->from = square;
    draggingState->isDragging = true;
}

void clickedChessBoard(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        chessBoardMouseButtonDown(&app.state->gameState, &app.state->draggingState);
        break;
    case SDL_MOUSEBUTTONUP:
        chessBoardMouseButtonUp(app);
        break;
    default:
        break;
    }
}

void clickedBackButton(SDL_Event event, App app) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        if (app.state->gameState.undoStates.previousStateIndex <= 1) return; // We cannot go back

        // Note that the time controls will not be updated because this is just for debugging purposes
        ChessPosition previousPos = app.state->gameState.undoStates.previousStates[--app.state->gameState.undoStates.previousStateIndex];
        RepetitionTable_pop();
        if (previousPos.colorToGo == app.state->gameState.playerColor) {
            app.state->gameState.currentPosition = previousPos;
        }
        else {
            app.state->gameState.currentPosition = app.state->gameState.undoStates.previousStates[--app.state->gameState.undoStates.previousStateIndex];
            RepetitionTable_pop();
        }

        break;
    default: // Only do something for mouse button down
        break;
    }
}

void clickedCopyFenButton(SDL_Event event, App app) {
    char fen[MAX_FEN_STRING_SIZE];
    int clipboardReturnValue;

    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        clipboardReturnValue = 0;
        if (event.button.button == SDL_BUTTON_LEFT) {
            FenString_chessPositionToFenString(app.state->gameState.currentPosition, fen);
            clipboardReturnValue = SDL_SetClipboardText(fen);
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
            if (app.state->gameState.undoStates.previousStateIndex > 0) {
                FenString_chessPositionToFenString(app.state->gameState.undoStates.previousStates[app.state->gameState.undoStates.previousStateIndex - 1], fen);
                clipboardReturnValue = SDL_SetClipboardText(fen);
            }
        }
        // I am adding this print statement so that even if the clipboard does not work we can still copy the fen string
        printf("Fen: %s\n", fen);
        if (clipboardReturnValue) {
            printf("Error setting clipboard: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
        break;
    default: // Only do something for mouse button down
        break;
    }
}
