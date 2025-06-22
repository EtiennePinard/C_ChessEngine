#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/FenString.h"
#include "../../sdl_framework/AppInit.h"

#include "../render/MainMenu.h"
#include "../render/GameScene.h"
#include "GameEvents.h"
#include "MainMenuEvents.h"

SDL_AppResult clickedDownWhitePlayerType(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->gameInfo.white.isEngine = !data->gameInfo.white.isEngine;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownBlackPlayerType(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->gameInfo.black.isEngine = !data->gameInfo.black.isEngine;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

char* onEnginePathSelected(const char* const* filelist) {
    // Handle error
    if (filelist == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "File dialog error: %s", SDL_GetError());
        return NULL;
    }

    // Handle cancel
    if (filelist[0] == NULL) {
        SDL_Log("File dialog canceled.");
        return NULL;
    }

    // Use the first selected file
    size_t length = strlen(filelist[0]);
    char* result = malloc(length * sizeof(char) + 1);
    strncpy(result, filelist[0], length);
    result[length] = '\0';
    return result;
}

static void onWhiteEnginePathSelected(void* userdata, const char* const* filelist, int filterIndex) {
    (void)filterIndex;

    App* app = (App*)userdata;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    // Re-enabling events and rendering
    app->events.shouldHandleEvents = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    char* enginePath = onEnginePathSelected(filelist);
    if (!enginePath) return;

    if (data->gameInfo.white.enginePath != NULL) free(data->gameInfo.white.enginePath);
    data->gameInfo.white.enginePath = enginePath;
}

static void onBlackEnginePathSelected(void* userdata, const char* const* filelist, int filterIndex) {
    (void)filterIndex;

    App* app = (App*)userdata;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    // Re-enabling events and rendering
    app->events.shouldHandleEvents = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    char* enginePath = onEnginePathSelected(filelist);
    if (!enginePath) return;

    if (data->gameInfo.black.enginePath != NULL) free(data->gameInfo.black.enginePath);
    data->gameInfo.black.enginePath = enginePath;
}

SDL_AppResult clickedDownEnginePath(App* app, PlayerConfig player, SDL_DialogFileCallback callback) {
    if (!player.isEngine) return SDL_APP_CONTINUE;

    SDL_DialogFileFilter filters[] = { { "All Files", "*" } };

    SDL_ShowOpenFileDialog(
        callback,
        app,
        app->state.sdlState.window,
        filters, 1,
        NULL,
        false
    );

    // Disabling events and rendering
    app->events.shouldHandleEvents = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, NO_RERENDER);

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownWhiteEnginePath(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    return clickedDownEnginePath(app, ((MainMenuSceneData*)app->state.currentScene.data)->gameInfo.white, &onWhiteEnginePathSelected);
}

SDL_AppResult clickedDownBlackEnginePath(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    return clickedDownEnginePath(app, ((MainMenuSceneData*)app->state.currentScene.data)->gameInfo.black, &onBlackEnginePathSelected);
}

SDL_AppResult clickedDownTimeControlButton(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->timeControlSettings.selectModalVisible = true;
    // Setting the hovered time left to 0 since we did not select any time control yet
    data->timeControlSettings.hovered.timeLeft = 0;

    app->state.currentScene.selectedRenderBoxIndex = TIME_CONTROL_MODAL;
    app->events.lockSelectedBoxIndex = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownTimeControlModal(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    // If the modal is not visible we don't do anything
    if (!data->timeControlSettings.selectModalVisible) return SDL_APP_CONTINUE;
    // A timeleft of 0 means no time controls were selected
    if (data->timeControlSettings.hovered.timeLeft == 0) return SDL_APP_CONTINUE;

    data->gameInfo.timeControl = data->timeControlSettings.hovered;
    data->timeControlSettings.selectModalVisible = false;

    app->state.currentScene.selectedRenderBoxIndex = TIME_CONTROL_BUTTON;
    app->events.lockSelectedBoxIndex = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedUpStartGame(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
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

    gameData->gameEndedInfo.result = GAME_IS_NOT_DONE;

    char* fenString = INITIAL_FEN;
    if (!FenString_setChessPositionFromCopiedFenString(fenString, &gameData->state.position)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the position\n");
        return SDL_APP_FAILURE;
    }

    gameData->state.white.timeControl = mainMenuData->gameInfo.timeControl;
    if (mainMenuData->gameInfo.white.isEngine) {
        gameData->state.white.engineCommunication = UCIEngine_initialize(mainMenuData->gameInfo.white.enginePath, "uci_engine_log_white.txt");
        if (!gameData->state.white.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for white at path `%s`\n", mainMenuData->gameInfo.white.enginePath);
            return SDL_APP_FAILURE;
        }
    }
    else {
        gameData->state.white.engineCommunication = NULL;
    }

    gameData->state.black.timeControl = mainMenuData->gameInfo.timeControl;
    if (mainMenuData->gameInfo.black.isEngine) {
        gameData->state.black.engineCommunication = UCIEngine_initialize(mainMenuData->gameInfo.black.enginePath, "uci_engine_log_black.txt");
        if (!gameData->state.black.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for black at path `%s`\n", mainMenuData->gameInfo.black.enginePath);
            return SDL_APP_FAILURE;
        }
    }
    else {
        gameData->state.black.engineCommunication = NULL;
    }

    gameData->promotionInfo.renderPromotionOverlay = false;
    gameData->gameEndedInfo.renderOverlay = false;

    gameData->selectedSquare.selectedSquare = (Square)-1;

    gameData->gameInfo = mainMenuData->gameInfo;

    // Calling the main menu terminating scene function
    app->state.currentScene.terminateSceneFunction(mainMenuData);

    // Setting the current scene to the game scene
    app->state.currentScene.sceneId = GAME_SCENE_ID;
    app->state.currentScene.data = gameData;
    app->state.currentScene.terminateSceneFunction = &terminateGameScene;
    computeGameSceneRender(app->state.sdlState.window, &app->state.currentScene);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    app->state.currentScene.selectedRenderBoxIndex = -1;
    app->events.mouseState.hoveredIndex = -1;

    
    ChessPosition dummyPosition = { 0 };
    UndoGameState undoState = {
        .position = dummyPosition,
        .playerToGoTimeControl = gameData->gameInfo.timeControl
    };
    // We append the timecontrol for the player to go
    da_append((&gameData->undoGameStates), undoState);
    
    resetGame(gameData);
    return SDL_APP_CONTINUE;
}
