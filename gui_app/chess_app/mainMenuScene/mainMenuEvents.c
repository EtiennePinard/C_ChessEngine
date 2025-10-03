#include "../../../engine/src/utils/FenString.h"

#include "../../sdl_framework/AppInit.h"
#include "../../sdl_framework/CommonEvents.h"
#include "../../sdl_framework/TextInput.h"
#include "../../sdl_framework/AppCleanup.h"

#include "../AppStyle.h"
#include "../gameScene/GameRender.h"
#include "../gameScene/GameEvents.h"

#include "MainMenuRender.h"
#include "MainMenuModals.h"
#include "MainMenuTextInput.h"
#include "MainMenuEvents.h"

SDL_AppResult startGame(App* app) {
    MainMenuSceneData* mainMenuData = (MainMenuSceneData*)app->state.currentScene.data;

    GameSceneData* gameData = SDL_calloc(1, sizeof(GameSceneData));
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

    // Loading the new game info
    gameData->gameInfo = mainMenuData->gameInfo;

    // Setting the current scene to the game scene
    app->state.currentScene.sceneId = GAME_SCENE_ID;
    app->state.currentScene.data = gameData;

    LoadGameInfoResult loadGameInfoResult = loadGameInfo(app);

    size_t messageSize;
    char* message;
    switch (loadGameInfoResult) {
    case INVALID_FEN_STRING:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the position\n");
        cleanupTextures(gameData->textures);
        SDL_free(gameData->textures.data);
        SDL_free(gameData);
        return SDL_APP_FAILURE;

    case EMPTY_WHITE_ENGINE_PATH:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Empty white engine path", "The white engine path is not set", app->state.sdlState.window);
        cleanupTextures(gameData->textures);
        SDL_free(gameData->textures.data);
        SDL_free(gameData);
        return SDL_APP_CONTINUE;

    case INVALID_WHITE_ENGINE_PATH:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for white at path `%s`\n", mainMenuData->gameInfo.white.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", mainMenuData->gameInfo.white.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", mainMenuData->gameInfo.white.engineConfig.enginePath);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid white engine path", message, app->state.sdlState.window);
        SDL_free(message);
        cleanupTextures(gameData->textures);
        SDL_free(gameData->textures.data);
        SDL_free(gameData);
        return SDL_APP_CONTINUE;

    case EMPTY_BLACK_ENGINE_PATH:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Empty black engine path", "The black engine path is not set", app->state.sdlState.window);
        cleanupTextures(gameData->textures);
        SDL_free(gameData->textures.data);
        SDL_free(gameData);
        return SDL_APP_CONTINUE;

    case INVALID_BLACK_ENGINE_PATH:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for black at path `%s`\n", mainMenuData->gameInfo.black.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", mainMenuData->gameInfo.black.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", mainMenuData->gameInfo.black.engineConfig.enginePath);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid black engine path", message, app->state.sdlState.window);
        SDL_free(message);
        cleanupTextures(gameData->textures);
        SDL_free(gameData->textures.data);
        SDL_free(gameData);
        return SDL_APP_CONTINUE;

    case SUCCESS:
    default:
        break;
    }

    // Calling the main menu terminating scene function
    app->state.currentScene.terminateSceneFunction(mainMenuData);

    // Setting the new terminateSceneFunction
    app->state.currentScene.terminateSceneFunction = &terminateGameScene;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    app->state.currentScene.selectedRenderBoxIndex = -1;
    app->events.mouseState.hoveredIndex = -1;

    resetGame(app);

    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        SDL_free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }
    return computeGameSceneRender(app);
}

SDL_AppResult mainMenuKeyDownEvent(SDL_Event* event, App* app) {
    if (event->key.key == SDLK_RETURN &&
        !app->events.modal.isActive &&
        !app->events.textInput.isActive) {
        // Start the game
        return startGame(app);
    }
    return SDL_APP_CONTINUE;
}

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
    return startGame(app);
}
