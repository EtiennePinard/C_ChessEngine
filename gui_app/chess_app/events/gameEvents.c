#include <stdio.h>
#include <stdlib.h>

#include "../../sdl_framework/EventHandler.h"

#include "../../../engine/src/state/Board.h"
#include "../../../engine/src/state/Move.h"
#include "../../../engine/src/moveHandler/MoveGenerator.h"
#include "../../../engine/src/moveHandler/MovePlayer.h"
#include "../../../engine/src/bot/RepetitionTable.h"
#include "../../../engine/src/utils/Math.h"
#include "../../../engine/src/utils/FenString.h"

#include "../uciEngineCommunication/UCIEngineCommunication.h"
#include "../render/GameScene.h"
#include "GameEvents.h"

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
    u64 piecesBitBoard = Board_allPiecesBitBoard(gameState->position.board);
    int nbPieces = numBitSet_64(piecesBitBoard);
    if (nbPieces > 4) { return false; }

    u64 rooksPawnsQueens = piecesBitBoard & (
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, QUEEN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, QUEEN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, ROOK)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, ROOK)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, PAWN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, PAWN))
        );

    if (rooksPawnsQueens != (u64)0) {
        // There is still pawns, rooks and/or queens on the board
        return false;
    }

    if (nbPieces <= 3) {
        // Lone bishop or lone knight (nbPiece == 3) or two lone kings (nbPiece == 2)
        return true;
    }

    int nbKWhiteKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, KNIGHT)));
    int nbKBlackKnights = numBitSet_64(piecesBitBoard & Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, KNIGHT)));
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
    Player currentPlayer = gameState->position.colorToGo == WHITE ? gameState->white : gameState->black;
    if (currentPlayer.timeControl.timeLeft <= (TimeControl_MS)0) {
        gameState->gameEndedSettings.result = gameState->position.colorToGo == WHITE ? BLACK_WON_ON_TIME : WHITE_WON_ON_TIME;
        return;
    }

    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMove;
    MoveHandler_getValidMoves(moves, &numMove, gameState->position);
    if (numMove == 0) {
        if (MoveHandler_isKingInCheck() || MoveHandler_isKingInDoubleCheck()) {
            gameState->gameEndedSettings.result = gameState->position.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        }
        else {
            gameState->gameEndedSettings.result = STALEMATE;
        }
    }
    else if (RepetitionTable_isKeyContainedTwiceInTable(gameState->position.key)) {
        gameState->gameEndedSettings.result = THREE_MOVE_REPETITION;
    }
    else if (gameState->position.turnsForFiftyRule > 50) {
        // TODO: Check this case out
        gameState->gameEndedSettings.result = FIFTY_MOVE_RULE;
    }
    else if (insufficientMaterialScenario(gameState)) {
        gameState->gameEndedSettings.result = INSUFFICIENT_MATERIAL;
    }
}

static inline void playMoveOnBoard(GameState* gameState, Move move) {
    if (gameState->undoStates.previousStateIndex >= gameState->undoStates.previousStateCapacity) {
        gameState->undoStates.previousStateCapacity *= 2;
        gameState->undoStates.previousStates = realloc(gameState->undoStates.previousStates, sizeof(GameState) * gameState->undoStates.previousStateCapacity);
        gameState->movesPlayed = realloc(gameState->movesPlayed, sizeof(Move) * gameState->undoStates.previousStateCapacity);
    }
    gameState->undoStates.previousStates[gameState->undoStates.previousStateIndex] = gameState->position;
    gameState->movesPlayed[gameState->undoStates.previousStateIndex] = move;
    gameState->undoStates.previousStateIndex++;

    Player* currentPlayer = gameState->position.colorToGo == WHITE ? &gameState->white : &gameState->black;
    currentPlayer->timeControl.timeLeft += currentPlayer->timeControl.increment;
    MoveHandler_playMove(move, &gameState->position, true);
    computeGameEnd(gameState);
    if (gameState->gameEndedSettings.result != GAME_IS_NOT_DONE) {
        gameState->gameEndedSettings.renderOverlay = true;
    }
}

static int botMove(void* app_pointer) {
    App* app = (App*)app_pointer;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    GameState* gameState = &data->state;

    ChessPosition startingPosition = (gameState->undoStates.previousStateIndex == 0) ?
        gameState->position :
        gameState->undoStates.previousStates[0];
    Player* currentPlayer = gameState->position.colorToGo == WHITE ? &gameState->white : &gameState->black;
    SDL_assert(currentPlayer->engineCommunication != NULL);

    Move botMove = UCIEngine_bestMoveTimed(
        currentPlayer->engineCommunication,
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

    if (Move_fromSquare(botMove) == Move_toSquare(botMove) && gameState->gameEndedSettings.result == GAME_IS_NOT_DONE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: The engine gave back a NULL_MOVE and the game is not done\n");
        exit(EXIT_FAILURE);
        return 1;
    }

    botMove = MoveHandler_correctMoveFlag(gameState->position, botMove);
    SDL_SetAtomicInt(&currentPlayer->isBotThinking, false);

    playMoveOnBoard(gameState, botMove);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, OTHER_THREAD_RERENDER);

    return 0;
}

SDL_Thread* playBotMove(App* app) {
    SDL_Thread* thread = SDL_CreateThread(botMove, "botMove", app);
    if (thread == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create thread: %s\n", SDL_GetError());
        return NULL;
    }
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    Player* currentPlayer = data->state.position.colorToGo == WHITE ? &data->state.white : &data->state.black;
    SDL_SetAtomicInt(&currentPlayer->isBotThinking, true);
    return thread;
}

SDL_AppResult resetGame(GameSceneData* data) {
    GameState* gameState = &data->state;
    // If we are not already at the beginning go back to the beginning
    ChessPosition startingPosition = (gameState->undoStates.previousStateIndex == 0) ?
        gameState->position :
        gameState->undoStates.previousStates[0];
    memcpy(&gameState->position, &startingPosition, sizeof(startingPosition));
    gameState->undoStates.previousStateIndex = 0;
    gameState->gameEndedSettings.result = GAME_IS_NOT_DONE;
    RepetitionTable_clear();

    gameState->white.timeControl = data->gameSettings.timeControl;
    if (data->gameSettings.white.isEngine) UCIEngine_sendCommand(gameState->white.engineCommunication, "ucinewgame");

    gameState->black.timeControl = data->gameSettings.timeControl;
    if (data->gameSettings.black.isEngine) UCIEngine_sendCommand(gameState->black.engineCommunication, "ucinewgame");

    gameState->previousTick = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

void clickedWhenGameIsDone(App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    data->state.gameEndedSettings.renderOverlay = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}

SDL_AppResult clickedDownRestartButton(SDL_Event* event, SDL_Rect rect, App* app) {
    (void) event;
    (void) rect;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (data->state.gameEndedSettings.result != GAME_IS_NOT_DONE) clickedWhenGameIsDone(app);
    resetGame(data);
    return SDL_APP_CONTINUE;
}

SDL_AppResult promotionOverlayMouseButtonDown(SDL_Event* event, SDL_Rect boardRect, App* app) {
    (void)boardRect;

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    // If the overlay is not visible we don't do anything
    if (!data->promotionSettings.renderPromotionOverlay) return SDL_APP_CONTINUE;

    // We need this if statement to invalidate the first click to the promotion overlay since
    // this first click could be the second click of the pawn moving to the promotion square
    if (data->selectedPiece.isPieceSelected) {
        // We are not holding the mouse button anymore
        data->selectedPiece.isPieceSelected = false;
        return SDL_APP_CONTINUE;
    }

    SDL_Rect overlayRect = data->promotionSettings.overlayRect;

    SDL_Point mousePoint = { (int)event->button.x, (int)event->button.y };
    // If we are not in the promotion overlay simply continue the app
    if (!SDL_PointInRect(&mousePoint, &overlayRect)) return SDL_APP_CONTINUE;

    Move move = NULL_MOVE;
    int squareSize = (STARTING_WINDOW_WIDTH * 2 / 3) / BOARD_LENGTH;
    int relativeX = ((int)event->button.x) - overlayRect.x;
    int relativeY = ((int)event->button.y) - overlayRect.y;

    int colIndex = relativeX / squareSize; // 0 or 1
    int rowIndex = relativeY / squareSize; // 0 or 1

    // Map the row and column to a piece
    int pieceIndex = rowIndex * 2 + colIndex;
    Square from = data->promotionSettings.promotionSquareFrom;
    Square to = data->promotionSettings.promotionSquareTo;
    switch (pieceIndex) {
    case 0: move = Move_makeMove(from, to, PROMOTE_TO_QUEEN); break;
    case 1: move = Move_makeMove(from, to, PROMOTE_TO_KNIGHT); break;
    case 2: move = Move_makeMove(from, to, PROMOTE_TO_ROOK); break;
    case 3: move = Move_makeMove(from, to, PROMOTE_TO_BISHOP); break;
    default: break;
    }

    if (move == NULL_MOVE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not select the promotion move\n");
        return SDL_APP_FAILURE;
    }

    playMoveOnBoard(&data->state, move);
    data->promotionSettings.renderPromotionOverlay = false;

    app->state.currentScene.selectedRenderBoxIndex = CHESSBOARD;
    app->events.lockSelectedBoxIndex = false;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult findAndPlayHumanMove(App* app, Square draggingTo) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    // Finding the valid moves of this position
    // We could cache this value if it really is that slow, but I don't think so
    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMoves;
    MoveHandler_getValidMoves(moves, &numMoves, data->state.position);

    for (int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        Move move = moves[moveIndex];

        if (Move_fromSquare(move) == data->selectedPiece.from && Move_toSquare(move) == draggingTo) {
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square (except for promotion)
            if (Move_flag(move) == PROMOTE_TO_QUEEN ||
                Move_flag(move) == PROMOTE_TO_KNIGHT ||
                Move_flag(move) == PROMOTE_TO_ROOK ||
                Move_flag(move) == PROMOTE_TO_BISHOP) {

                data->promotionSettings.renderPromotionOverlay = true;
                data->promotionSettings.promotionSquareFrom = data->selectedPiece.from;
                data->promotionSettings.promotionSquareTo = draggingTo;

                app->state.currentScene.selectedRenderBoxIndex = PROMOTION_OVERLAY;
                app->events.lockSelectedBoxIndex = true;
                // The promotion settings will take care of playing the chosen move on the board
            }
            else {
                playMoveOnBoard(&data->state, move);
            }
            break;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult chessBoardMouseButtonUp(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (data->state.gameEndedSettings.result != GAME_IS_NOT_DONE) {
        clickedWhenGameIsDone(app);
        return SDL_APP_CONTINUE;
    }

    Player currentPlayer = data->state.position.colorToGo == WHITE ? data->state.white : data->state.black;

    // We are dragging a piece from the opposite color. Also if gameScene.selectedPiece.draggedPiece 
    // is NOPIECE than Piece_color will evaluate to 0 and colorToGo cannot be 0 
    // Furthermore if the currentPlayer is an engine, let the engine think
    if (data->state.position.colorToGo != Piece_color(data->selectedPiece.selectedPiece) || currentPlayer.engineCommunication != NULL) {
        // Reset the dragging state
        memset(&data->selectedPiece, 0, sizeof(SelectedPiece));
        return SDL_APP_CONTINUE;
    }

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    Square draggingTo = squareFromxy((int)mouseX, (int)mouseY, data->flipBoard, rect);

    if (draggingTo != data->selectedPiece.from) {
        // We dragged the piece to a square other than its starting square
        // we are not dragging anymore and we find the move the player wants to play
        SDL_AppResult result = findAndPlayHumanMove(app, draggingTo);
        if (result != SDL_APP_CONTINUE) return result;
        data->selectedPiece.isPieceSelected = false;
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }
    else {
        data->selectedPiece.isDragged = false;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult chessBoardMouseButtonDown(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (data->state.gameEndedSettings.result != GAME_IS_NOT_DONE) {
        clickedWhenGameIsDone(app);
        return SDL_APP_CONTINUE;
    }

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    int square = squareFromxy((int)mouseX, (int)mouseY, data->flipBoard, rect);

    if (data->selectedPiece.isPieceSelected) {
        SDL_assert(!data->selectedPiece.isDragged);

        // We are already "dragging" a piece
        Square draggingTo = squareFromxy((int)mouseX, (int)mouseY, data->flipBoard, rect);
        Piece draggingToPiece = Board_pieceAtIndex(data->state.position.board, draggingTo);
        Piece draggingFromPiece = Board_pieceAtIndex(data->state.position.board, data->selectedPiece.from);
        if (Piece_color(draggingToPiece) == Piece_color(draggingFromPiece)) {
            // The player does not want to move the piece
            data->selectedPiece.selectedPiece = draggingToPiece;
            data->selectedPiece.from = draggingTo;
            data->selectedPiece.isDragged = true;
        }
        else {
            SDL_AppResult result = findAndPlayHumanMove(app, draggingTo);
            if (result != SDL_APP_CONTINUE) return result;
            data->selectedPiece.isPieceSelected = false;
        }
    }
    else {
        data->selectedPiece.selectedPiece = Board_pieceAtIndex(data->state.position.board, square);
        data->selectedPiece.from = square;
        data->selectedPiece.isPieceSelected = true;
        data->selectedPiece.isDragged = true;
    }

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

// void clickedRewindButton(SDL_Event event, App app) {
//     switch (event.type) {
//     case SDL_EVENT_MOUSE_BUTTON_DOWN:
//         if (app.state.gameScene.state.undoStates.previousStateIndex <= 1) return; // We cannot go back

//         // Note that the time controls will not be updated because this is just for debugging purposes
//         ChessPosition previousPos = app.state.gameScene.state.undoStates.previousStates[--app.state.gameScene.state.undoStates.previousStateIndex];
//         RepetitionTable_pop();
//         if (previousPos.colorToGo == app.state.gameScene.state.playerColor) {
//             app.state.gameScene.state.position = previousPos;
//         }
//         else {
//             app.state.gameScene.state.position = app.state.gameScene.state.undoStates.previousStates[--app.state.gameScene.state.undoStates.previousStateIndex];
//             RepetitionTable_pop();
//         }

//         break;
//     default: // Only do something for mouse button down
//         break;
//     }
// }
