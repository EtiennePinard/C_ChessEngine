#include <stdio.h>
#include <stdlib.h>

#include "../../../engine/src/utils/FenString.h"

#include "../../sdl_framework/AppInit.h"
#include "../../sdl_framework/CommonEvents.h"
#include "../../sdl_framework/TextInput.h"

#include "../AppStyle.h"
#include "../gameScene/GameRender.h"
#include "../gameScene/GameEvents.h"

#include "MainMenuRender.h"
#include "MainMenuModals.h"
#include "MainMenuTextInput.h"
#include "MainMenuEvents.h"

SDL_AppResult enteredStartingPosition(SDL_Event* event, SDL_FRect rect, App* app) {
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return changeMouseIconOnEnterTextInput(event, rect, app);
}

SDL_AppResult exitedStartingPosition(SDL_Event* event, SDL_FRect rect, App* app) {
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return resetMouseIconOnExitTextInput(event, rect, app);
}

SDL_AppResult clickedStartingPosition(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;

    // If the text input is already active, do nothing
    if (app->events.textInput.isActive) return SDL_APP_CONTINUE;

    setStartingPositionTextInputActive(rect, app);
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
