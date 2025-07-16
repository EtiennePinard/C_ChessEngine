#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/FenString.h"

#include "../../sdl_framework/AppInit.h"
#include "../../sdl_framework/CommonEvents.h"

#include "../render/scene/MainMenuScene.h"
#include "../render/modal/MainMenuModals.h"
#include "../render/scene/GameScene.h"
#include "GameEvents.h"
#include "MainMenuEvents.h"

// TODO: Figure out of the framework will handle errors on close 
// and code included in the framework and not included
SDL_AppResult startingPositionTextInputReturn(SDL_Event* event, App* app) {
    (void)event;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    ChessPosition dummyPosition;
    char fenCopy[app->events.textInput.text.count + 1];
    SDL_strlcpy(fenCopy, app->events.textInput.text.data, app->events.textInput.text.count + 1);
    if (!FenString_setChessPositionFromFenString(fenCopy, &dummyPosition)) {
        SDL_Log("The inputted fen string, `%s`, is incorrect\n", app->events.textInput.text.data);
        int messageSize = snprintf(NULL, 0, "The inputted fen string, `%s`, is not a valid fen string", app->events.textInput.text.data) + 1;
        char message[messageSize];
        snprintf(message, messageSize, "The inputted fen string, `%s` is not a valid fen string", app->events.textInput.text.data);
        // TODO: Bug when clicking the return key to exit the popup
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fen string error", message, app->state.sdlState.window);
    }
    else {
        // The fen string is correct, store it into the gameInfo field
        if (data->gameInfo.startingPositionFen) free(data->gameInfo.startingPositionFen);
        data->gameInfo.startingPositionFen = calloc(app->events.textInput.text.count + 1, sizeof(char));
        SDL_assert(data->gameInfo.startingPositionFen);
        SDL_strlcpy(data->gameInfo.startingPositionFen, app->events.textInput.text.data, app->events.textInput.text.count + 1);

        free(app->events.textInput.text.data);
        app->events.textInput.text.data = NULL;

        SDL_StopTextInput(app->state.sdlState.window);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedStartingPosition(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;

    // If the text input is already active simply return
    if (app->events.textInput.isActive) return SDL_APP_CONTINUE;

    SDL_Rect area = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
    app->events.textInput.text.capacity = MAX_FEN_STRING_SIZE;
    app->events.textInput.text.count = 0;
    // we are guaranteed that the text data is either already freed, or transferred
    // to the gameInfo field
    app->events.textInput.text.data = calloc(app->events.textInput.text.capacity, sizeof(char));
    SDL_assert(app->events.textInput.text.data);

    app->events.textInput.onEscape = &closeTextInput;
    app->events.textInput.onReturn = &startingPositionTextInputReturn;
    
    app->events.textInput.keepOnlyAscii = true;
    app->events.textInput.isActive = true;

    app->events.textInput.cursorIndex = 0;

    SDL_SetTextInputArea(app->state.sdlState.window,
        &area,
        area.w / 2);
    SDL_StartTextInput(app->state.sdlState.window);

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedWhiteEngineConfig(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    setEngineConfigModalActive(app, WHITE);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedBlackEngineConfig(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    setEngineConfigModalActive(app, BLACK);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedWhiteTimeControl(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    setTimeControlModalActive(app, WHITE);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedBlackTimeControl(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    setTimeControlModalActive(app, BLACK);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedStartGame(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    MainMenuSceneData* mainMenuData = (MainMenuSceneData*)app->state.currentScene.data;

    GameSceneData* gameData = calloc(1, sizeof(GameSceneData));
    gameData->flipBoard = false; // We don't have an option for that yet

    const char* filePaths[NB_PIECES] = {
        WHITE_PAWN_IMG_PATH, WHITE_KNIGHT_IMG_PATH, WHITE_BISHOP_IMG_PATH, WHITE_ROOK_IMG_PATH, WHITE_QUEEN_IMG_PATH, WHITE_KING_IMG_PATH,
        BLACK_PAWN_IMG_PATH, BLACK_KNIGHT_IMG_PATH, BLACK_BISHOP_IMG_PATH, BLACK_ROOK_IMG_PATH, BLACK_QUEEN_IMG_PATH, BLACK_KING_IMG_PATH
    };

    if (!initializeTextures(&gameData->textures, NB_PIECES) ||
        !loadImageFromFilePath(&app->state.sdlState, &gameData->textures, filePaths, NB_PIECES)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the chess images\n");
        return SDL_APP_FAILURE;
    }

    gameData->state.result = GAME_IS_NOT_DONE;

    if (!FenString_setChessPositionFromCopiedFenString(mainMenuData->gameInfo.startingPositionFen, &gameData->state.position)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the position\n");
        return SDL_APP_FAILURE;
    }

    gameData->state.white.timeControl = mainMenuData->gameInfo.white.timeControl;
    if (mainMenuData->gameInfo.white.engineConfig.isEngine) {
        gameData->state.white.engineCommunication = UCIEngine_initialize(mainMenuData->gameInfo.white.engineConfig.enginePath, "uci_engine_log_white.txt");
        if (!gameData->state.white.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for white at path `%s`\n", mainMenuData->gameInfo.white.engineConfig.enginePath);
            return SDL_APP_FAILURE;
        }
    }
    else {
        gameData->state.white.engineCommunication = NULL;
    }

    gameData->state.black.timeControl = mainMenuData->gameInfo.black.timeControl;
    if (mainMenuData->gameInfo.black.engineConfig.isEngine) {
        gameData->state.black.engineCommunication = UCIEngine_initialize(mainMenuData->gameInfo.black.engineConfig.enginePath, "uci_engine_log_black.txt");
        if (!gameData->state.black.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for black at path `%s`\n", mainMenuData->gameInfo.black.engineConfig.enginePath);
            return SDL_APP_FAILURE;
        }
    }
    else {
        gameData->state.black.engineCommunication = NULL;
    }

    gameData->selectedSquare.selectedSquare = NO_SQUARE_SELECTED;

    gameData->gameInfo = mainMenuData->gameInfo;

    // Calling the main menu terminating scene function
    app->state.currentScene.terminateSceneFunction(mainMenuData);

    // Setting the current scene to the game scene
    app->state.currentScene.sceneId = GAME_SCENE_ID;
    app->state.currentScene.data = gameData;
    app->state.currentScene.terminateSceneFunction = &terminateGameScene;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    app->state.currentScene.selectedRenderBoxIndex = -1;
    app->events.mouseState.hoveredIndex = -1;

    resetGame(event, app);

    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }
    return computeGameSceneRender(app);
}
