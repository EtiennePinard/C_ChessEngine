#include "../../../engine/src/state/Board.h"
#include "../../../engine/src/state/Move.h"
#include "../../../engine/src/moveHandler/MoveGenerator.h"
#include "../../../engine/src/moveHandler/MovePlayer.h"
#include "../../../engine/src/bot/RepetitionTable.h"
#include "../../../engine/src/utils/Math.h"
#include "../../../engine/src/utils/FenString.h"

#include "../../sdl_framework/AppCleanup.h"
#include "../../sdl_framework/AppInit.h"

#include "../uciEngineCommunication/UCIEngineCommunication.h"
#include "GameModals.h"
#include "GameRender.h"
#include "../mainMenuScene/MainMenuRender.h"
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
    BitBoard piecesBitBoard = Board_allPiecesBitBoard(gameState->position.board);
    int nbPieces = numBitSet_64(piecesBitBoard);
    if (nbPieces > 4) { return false; }

    BitBoard rooksPawnsQueens = piecesBitBoard & (
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, QUEEN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, QUEEN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, ROOK)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, ROOK)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(WHITE, PAWN)) |
        Board_bitBoardForPiece(gameState->position.board, Piece_makePiece(BLACK, PAWN))
        );

    if (rooksPawnsQueens != (BitBoard)0) {
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
        gameState->result = gameState->position.colorToGo == WHITE ? BLACK_WON_ON_TIME : WHITE_WON_ON_TIME;
        return;
    }

    Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int numMove;
    MoveHandler_getValidMoves(moves, &numMove, gameState->position);
    if (numMove == 0) {
        if (MoveHandler_isKingInCheck() || MoveHandler_isKingInDoubleCheck()) {
            gameState->result = gameState->position.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        }
        else {
            gameState->result = STALEMATE;
        }
    }
    else if (RepetitionTable_isKeyContainedTwiceInTable(gameState->position.key)) {
        gameState->result = THREE_MOVE_REPETITION;
    }
    else if (gameState->position.turnsForFiftyRule > 50) {
        // TODO: Check this case out
        gameState->result = FIFTY_MOVE_RULE;
    }
    else if (insufficientMaterialScenario(gameState)) {
        gameState->result = INSUFFICIENT_MATERIAL;
    }
}

void playMoveOnBoard(GameSceneData* data, Move move) {
    GameState* gameState = &data->state;

    // We add the time increment
    // Note: We add the increment before checking if we have ran out of time
    // Maybe verify if this is how a gui is supposed to do it
    Player* currentPlayer = gameState->position.colorToGo == WHITE ? &gameState->white : &gameState->black;
    currentPlayer->timeControl.timeLeft += currentPlayer->timeControl.increment;
    MoveHandler_playMove(move, &gameState->position, true);

    // We append the timecontrol for the player to go
    UndoGameState undoState = {
        .position = gameState->position,
        .whiteTimeControl = gameState->white.timeControl,
        .blackTimeControl = gameState->black.timeControl
    };
    da_append((&data->undoGameStates), undoState);
    da_append((&data->moveListInfo.movesPlayed), move);

    computeGameEnd(gameState);
}

static int botMove(void* app_pointer) {
    App* app = (App*)app_pointer;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    GameState* gameState = &data->state;

    ChessPosition startingPosition = (data->undoGameStates.count <= 1) ?
        gameState->position :
        data->undoGameStates.data[0].position;
    Player* currentPlayer = gameState->position.colorToGo == WHITE ? &gameState->white : &gameState->black;
    SDL_assert(currentPlayer->engineCommunication != NULL);

    Move botMove = UCIEngine_bestMoveTimed(
        currentPlayer->engineCommunication,
        startingPosition,
        data->moveListInfo.movesPlayed.data,
        data->moveListInfo.movesPlayed.count,
        200
        // gameState->white.timeControl.timeLeft,
        // gameState->white.timeControl.timeLeft,
        // gameState->white.timeControl.increment,
        // gameState->black.timeControl.increment,
        // -1 // We don't have movesToGo for now
    );

    if (Move_fromSquare(botMove) == Move_toSquare(botMove) && data->state.result == GAME_IS_NOT_DONE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "The engine gave back a NULL_MOVE and the game is not done\n");
        return 1;
    }

    botMove = MoveHandler_correctMoveFlag(gameState->position, botMove);
    SDL_SetAtomicInt(&currentPlayer->isBotThinking, false);

    playMoveOnBoard(data, botMove);
    if (data->state.result != GAME_IS_NOT_DONE) setGameEndedModalActive(app);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, OTHER_THREAD_RERENDER);

    return 0;
}

SDL_Thread* playBotMove(App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    Player* currentPlayer = data->state.position.colorToGo == WHITE ? &data->state.white : &data->state.black;
    SDL_SetAtomicInt(&currentPlayer->isBotThinking, true);

    SDL_Thread* thread = SDL_CreateThread(botMove, "botMove", app);
    if (thread == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create thread: %s\n", SDL_GetError());
        SDL_SetAtomicInt(&currentPlayer->isBotThinking, false);
        return NULL;
    }
    return thread;
}

SDL_AppResult resetGame(SDL_Event* event, App* app) {
    if (app->events.modal.isActive) {
        // Cancel the modal
        if (app->events.modal.onEscape) app->events.modal.onEscape(event, app);
        app->events.modal.isActive = false;
    }

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    GameState* gameState = &data->state;
    // If we are not already at the beginning go back to the beginning
    ChessPosition startingPosition = (data->undoGameStates.count <= 1) ?
        gameState->position :
        data->undoGameStates.data[0].position;
    gameState->position = startingPosition;

    // Resetting the gameState
    if (data->undoGameStates.count > 0) {
        // If there is already some elements in the undo states simply return to the first one
        // which has the correct starting time control for the starting player of the position
        data->undoGameStates.count = 1;
    }
    else {
        // If no elements are in the undo states then add the first one
        // with the correct time control
        UndoGameState undoState = {
            .position = startingPosition,
            .whiteTimeControl = data->gameInfo.white.timeControl,
            .blackTimeControl = data->gameInfo.black.timeControl
        };
        // We append the timecontrol for the player to go
        da_append((&data->undoGameStates), undoState);
    }

    data->moveListInfo.movesPlayed.count = 0;
    data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
    data->moveListInfo.scrollRatio = 0.0f;
    data->state.result = GAME_IS_NOT_DONE;
    RepetitionTable_clear();

    gameState->white.timeControl = data->gameInfo.white.timeControl;
    if (data->gameInfo.white.engineConfig.isEngine) UCIEngine_sendCommand(gameState->white.engineCommunication, "ucinewgame");

    gameState->black.timeControl = data->gameInfo.black.timeControl;
    if (data->gameInfo.black.engineConfig.isEngine) UCIEngine_sendCommand(gameState->black.engineCommunication, "ucinewgame");

    gameState->previousTick = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

// TODO: Add a global click event callback
// void clickedWhenGameIsDone(App* app) {
//     GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
//     data->gameEndedInfo.renderOverlay = false;
//     SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
// }

SDL_AppResult clickedRestartButton(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)rect;
    return resetGame(event, app);
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

        if (Move_fromSquare(move) == data->selectedSquare.selectedSquare && Move_toSquare(move) == draggingTo) {
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square (except for promotion)
            if (Move_flag(move) == PROMOTE_TO_QUEEN ||
                Move_flag(move) == PROMOTE_TO_KNIGHT ||
                Move_flag(move) == PROMOTE_TO_ROOK ||
                Move_flag(move) == PROMOTE_TO_BISHOP) {

                setPromotionModalActive(app, draggingTo);
                // The promotion settings will take care of playing the chosen move on the board
            }
            else {
                playMoveOnBoard(data, move);
                // We played a move so we reset the selected square
                data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
            }
            break;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult chessBoardMouseButtonUp(SDL_Event* event, SDL_FRect rect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (data->state.result != GAME_IS_NOT_DONE) return SDL_APP_CONTINUE;

    if (data->selectedSquare.selectedSquare == NO_SQUARE_SELECTED) {
        // The user clicked down outside the board rect and then moved
        // the mouse inside the board rect and clicked up
        // In this case we simply return
        return SDL_APP_CONTINUE;
    }

    Piece selectedPiece = Board_pieceAtIndex(data->state.position.board, data->selectedSquare.selectedSquare);
    // If the selected square does not contain a square simply return
    if (selectedPiece == NO_PIECE) return SDL_APP_CONTINUE;

    Player currentPlayer = data->state.position.colorToGo == WHITE ? data->state.white : data->state.black;

    // We are dragging a piece from the opposite color. Also if gameScene.selectedPiece.draggedPiece 
    // is NO_PIECE than Piece_color will evaluate to 0 and colorToGo cannot be 0 
    // Furthermore if the currentPlayer is an engine, let the engine think
    if (data->state.position.colorToGo != Piece_color(selectedPiece) || currentPlayer.engineCommunication != NULL) {
        // Reset the selected piece
        data->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;
        return SDL_APP_CONTINUE;
    }

    Square draggingTo = squareFromxy((int)event->button.x, (int)event->button.y, data->flipBoard, rect);
    if (draggingTo == NO_SQUARE_SELECTED) {
        // Probably a floating point error when the mouse is on the edge of the board
        return SDL_APP_CONTINUE;
    }

    if (draggingTo != data->selectedSquare.selectedSquare) {
        // We dragged the piece to a square other than its starting square
        // we are not dragging anymore and we find the move the player wants to play
        SDL_AppResult result = findAndPlayHumanMove(app, draggingTo);
        if (result != SDL_APP_CONTINUE) return result;
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult chessBoardMouseButtonDown(SDL_Event* event, SDL_FRect rect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    if (data->state.result != GAME_IS_NOT_DONE) return SDL_APP_CONTINUE;
    
    Square square = squareFromxy((int)event->button.x, (int)event->button.y, data->flipBoard, rect);
    if (square == NO_SQUARE_SELECTED) {
        // Probably a floating point error when the mouse is on the edge of the board
        return SDL_APP_CONTINUE;
    }

    if (data->selectedSquare.selectedSquare == NO_SQUARE_SELECTED) {
        data->selectedSquare.selectedSquare = square;
    }
    else {
        Piece selectedPiece = Board_pieceAtIndex(data->state.position.board, data->selectedSquare.selectedSquare);
        Piece draggingToPiece = Board_pieceAtIndex(data->state.position.board, square);
        if (selectedPiece == NO_PIECE || Piece_color(draggingToPiece) == Piece_color(selectedPiece)) {
            // The player does not want to move the piece or selected an empty square
            data->selectedSquare.selectedSquare = square;
        }
        else {
            SDL_AppResult result = findAndPlayHumanMove(app, square);
            if (result != SDL_APP_CONTINUE) return result;
        }
    }

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownBackButton(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;
    (void)rect;

    GameSceneData* gameData = (GameSceneData*)app->state.currentScene.data;
    MainMenuSceneData* mainMenuData = SDL_calloc(1, sizeof(MainMenuSceneData));

    mainMenuData->gameInfo = gameData->gameInfo;
    app->events.modal.isActive = false;

    const char* mainMenuImages[2] = { HUMAN_ICON_PATH, COMPUTER_ICON_PATH };
    if (!initializeTextures(&mainMenuData->textures, 2) ||
        !loadImageFromFilePath(&app->state.sdlState, &mainMenuData->textures, mainMenuImages, 2)) {
        return false;
    }

    app->state.currentScene.selectedRenderBoxIndex = -1;
    app->events.mouseState.hoveredIndex = -1;

    // Calling the main menu terminating scene function
    app->state.currentScene.terminateSceneFunction(gameData);

    // Setting the current scene to the game scene
    app->state.currentScene.sceneId = MAIN_MENU_SCENE_ID;
    app->state.currentScene.data = mainMenuData;
    app->state.currentScene.terminateSceneFunction = &terminateMainMenuScene;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        SDL_free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }
    return computeMainMenuSceneRender(app);
}

SDL_AppResult clickedDownFlipBoardButton(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;
    (void)rect;

    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    data->flipBoard = !data->flipBoard;

    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        SDL_free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }
    if (computeGameSceneRender(app) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownMoveList(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;
    (void)rect;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

    int selectedIndex = data->moveListInfo.hoveredMoveIndex;
    // An index of -1 means no moves is selected
    if (selectedIndex == -1) return SDL_APP_CONTINUE;

    // Reset the game so that the previous move made is the move at the index
    data->state.result = GAME_IS_NOT_DONE;
    data->moveListInfo.movesPlayed.count = selectedIndex + 1;
    RepetitionTable_setIndex(selectedIndex + 1);

    data->undoGameStates.count = selectedIndex + 2;
    UndoGameState undoState = data->undoGameStates.data[data->undoGameStates.count - 1];
    data->state.position = undoState.position;
    data->state.white.timeControl = undoState.whiteTimeControl;
    data->state.black.timeControl = undoState.blackTimeControl;

    computeGameEnd(&data->state);
    if (data->state.result != GAME_IS_NOT_DONE) setGameEndedModalActive(app);
    
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    return SDL_APP_CONTINUE;
}

#define MAXIMUM_SCROLL_WHEEL_TICKS_AMOUNT (25.0) 

SDL_AppResult movelistMouseWheelScrolled(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)rect;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    // If no moves have been made don't scroll the scroll bar
    if (data->moveListInfo.movesPlayed.count == 0) return SDL_APP_CONTINUE;

    // We want the down direction on the mouse to go down
    const float scrolledAmount = -(event->wheel.y) / MAXIMUM_SCROLL_WHEEL_TICKS_AMOUNT;
    data->moveListInfo.scrollRatio = SDL_clamp(data->moveListInfo.scrollRatio + scrolledAmount, 0.0, 1.0);

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownScrollbar(SDL_Event* event, SDL_FRect rect, App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

    if (data->moveListInfo.movesPlayed.count == 0) return SDL_APP_CONTINUE;

    // We are scrolling the screen!
    // Note: This will be set to false the moment we stop holding click
    data->moveListInfo.isScrolling = true;

    const float maxScrollY = rect.h - data->moveListInfo.scrollbarFRect.h;
    SDL_FPoint mousePoint = { event->button.x, event->button.y };

    // If we click in the scrollbar do not set the scroll ratio but correctly set the dragoffset
    if (SDL_PointInRectFloat(&mousePoint, &data->moveListInfo.scrollbarFRect)) {
        data->moveListInfo.startingDragOffset = mousePoint.y - data->moveListInfo.scrollbarFRect.y;
        return SDL_APP_CONTINUE;
    }

    data->moveListInfo.startingDragOffset = data->moveListInfo.scrollbarFRect.h / 2.0;
    if (mousePoint.y >= rect.y + maxScrollY) {
        // We exceeded the max scroll ratio so we set it to 1.0
        data->moveListInfo.scrollRatio = 1.0;
    }
    else {
        data->moveListInfo.scrollRatio = (mousePoint.y - data->moveListInfo.startingDragOffset - rect.y) / maxScrollY;
    }
    return SDL_APP_CONTINUE;
}
