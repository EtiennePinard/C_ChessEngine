#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/FenString.h"
#include "../../sdl_framework/AppInit.h"
#include "../../sdl_framework/AppCleanup.h"
#include "../render/MainMenu.h"
#include "../render/GameScene.h"
#include "../Config.h"
#include "MainMenuEvents.h"

SDL_AppResult clickedDownWhitePlayerType(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->white.isEngine = !data->white.isEngine;
    app->state.currentScene.shouldRender = true;
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownBlackPlayerType(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->black.isEngine = !data->black.isEngine;
    app->state.currentScene.shouldRender = true;
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
    char* result = malloc(length * sizeof(char));
    strncpy(result, filelist[0], length);
    return result;
}

static void onWhiteEnginePathSelected(void* userdata, const char* const* filelist, int filterIndex) {
    (void)filterIndex;

    App* app = (App*)userdata;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    // Re-enabling events and rendering
    app->events.shouldHandleEvents = true;
    app->state.currentScene.shouldRender = true;

    char* enginePath = onEnginePathSelected(filelist);
    if (!enginePath) return;

    if (data->white.enginePath != NULL) free(data->white.enginePath);
    data->white.enginePath = enginePath;
}

static void onBlackEnginePathSelected(void* userdata, const char* const* filelist, int filterIndex) {
    (void)filterIndex;

    App* app = (App*)userdata;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    // Re-enabling events and rendering
    app->events.shouldHandleEvents = true;
    app->state.currentScene.shouldRender = true;

    char* enginePath = onEnginePathSelected(filelist);
    if (!enginePath) return;

    if (data->black.enginePath != NULL) free(data->black.enginePath);
    data->black.enginePath = enginePath;
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
    app->state.currentScene.shouldRender = false;

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownWhiteEnginePath(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    return clickedDownEnginePath(app, ((MainMenuSceneData*)app->state.currentScene.data)->white, &onWhiteEnginePathSelected);
}

SDL_AppResult clickedDownBlackEnginePath(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    return clickedDownEnginePath(app, ((MainMenuSceneData*)app->state.currentScene.data)->black, &onBlackEnginePathSelected);
}

SDL_AppResult clickedDownTimeControlButton(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    data->timeControl.selectModalVisible = true;
    // Setting the hovered time left to 0 since we did not select any time control yet
    data->timeControl.hovered.timeLeft = 0;

    app->state.currentScene.selectedRenderBoxIndex = TIME_CONTROL_MODAL;
    app->events.lockSelectedBoxIndex = true;
    app->state.currentScene.shouldRender = true;

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownTimeControlModal(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    // If the modal is not visible we don't do anything
    if (!data->timeControl.selectModalVisible) return SDL_APP_CONTINUE;
    // A timeleft of 0 means no time controls were selected
    if (data->timeControl.hovered.timeLeft == 0) return SDL_APP_CONTINUE;

    data->timeControl.selected = data->timeControl.hovered;
    data->timeControl.selectModalVisible = false;

    app->state.currentScene.selectedRenderBoxIndex = TIME_CONTROL_BUTTON;
    app->events.lockSelectedBoxIndex = false;
    app->state.currentScene.shouldRender = true;

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownStartGame(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    MainMenuSceneData* mainMenuData = (MainMenuSceneData*)app->state.currentScene.data;
    if (!saveMainMenuConfig(mainMenuData)) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error saving the config file\n");

    GameSceneData* gameData = calloc(1, sizeof(GameSceneData));
    gameData->flipBoard = false; // We don't have an option for that yet

    const char* filePaths[NB_PIECES] = {
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
    };

    if (!initializeTextures(&gameData->textures, NB_PIECES) ||
        !loadImageFromFilePath(&app->state.sdlState, &gameData->textures, filePaths, NB_PIECES)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the chess images\n");
        return SDL_APP_FAILURE;
    }

    gameData->state.result = GAME_IS_NOT_DONE;
    gameData->state.undoStates.previousStateCapacity = 64;
    gameData->state.undoStates.previousStates = malloc(sizeof(ChessPosition) * gameData->state.undoStates.previousStateCapacity);
    gameData->state.undoStates.previousStateIndex = 0;

    gameData->state.movesPlayed = malloc(sizeof(Move) * gameData->state.undoStates.previousStateCapacity);
    char* fenString = "k7/6P1/6K1/8/8/8/8/8 w - - 0 1";
    if (!FenString_setChessPositionFromCopiedFenString(fenString, &gameData->state.currentPosition)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the position\n");
        return SDL_APP_FAILURE;
    }

    gameData->state.white.remainingTime = mainMenuData->timeControl.selected.timeLeft;
    gameData->state.white.increment = mainMenuData->timeControl.selected.increment;
    if (mainMenuData->white.isEngine) {
        gameData->state.white.engineCommunication = UCIEngine_initialize(mainMenuData->white.enginePath, "uci_engine_log_white.txt");
        if (!gameData->state.white.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the engine at path `%s`\n", mainMenuData->white.enginePath);
            return SDL_APP_FAILURE;
        }
    } else {
        gameData->state.white.engineCommunication = NULL;
    }

    gameData->state.black.remainingTime = mainMenuData->timeControl.selected.timeLeft;
    gameData->state.black.increment = mainMenuData->timeControl.selected.increment;
    if (mainMenuData->black.isEngine) {
        gameData->state.black.engineCommunication = UCIEngine_initialize(mainMenuData->black.enginePath, "uci_engine_log_black.txt");
        if (!gameData->state.black.engineCommunication) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the engine at path `%s`\n", mainMenuData->black.enginePath);
            return SDL_APP_FAILURE;
        }
    } else {
        gameData->state.black.engineCommunication = NULL;
    }

    // We don't need this scene anymore
    cleanupTextures(mainMenuData->textures);
    free(mainMenuData->textures.data);
    free(mainMenuData->black.enginePath);
    free(mainMenuData->white.enginePath);
    free(mainMenuData);

    app->state.currentScene.sceneId = GAME_SCENE_ID;
    app->state.currentScene.shouldRender = true;
    app->state.currentScene.data = gameData;
    computeGameSceneRender(app->state.sdlState.window, &app->state.currentScene);   

    app->events.mouseState.hoveredIndex = -1;

    gameData->state.previousTick = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}
