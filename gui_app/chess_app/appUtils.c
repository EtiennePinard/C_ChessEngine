#include "../sdl_framework/CommonRenderFunctions.h"
#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/AppCleanup.h"

#include "gameScene/GameEvents.h"
#include "gameScene/GameRender.h"

#include "AppUtils.h"

SDL_AppResult setGameScene(App* app, GameConfig* gameConfig, AppStyle* appStyle) {
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

    // Setting game info and app style
    gameData->gameInfo = *gameConfig;
    gameData->appStyle = *appStyle;

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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for white at path `%s`\n", gameConfig->white.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", gameConfig->white.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", gameConfig->white.engineConfig.enginePath);
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
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for black at path `%s`\n", gameConfig->black.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", gameConfig->black.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", gameConfig->black.engineConfig.enginePath);
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

char* copyString(const char* stringToCopy) {
    if (!stringToCopy) return NULL;
    size_t length = SDL_strlen(stringToCopy) + 1;
    char* result = SDL_calloc(length, sizeof(char));
    SDL_assert(SDL_strlcpy(result, stringToCopy, length) == length - 1);
    return result;
}

SDL_AppResult formatTime(TimeControl_MS milliseconds, char* output, size_t outputSize) {
    if (!output || outputSize < 6) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter or outputSize less than 6 at " __FILE__);
        return SDL_APP_FAILURE;
    }

    u32 totalSeconds = milliseconds / 1000;
    u32 minutes = totalSeconds / 60;
    u32 seconds = totalSeconds % 60;
    // Format the string as "mm:ss"
    SDL_snprintf(output, outputSize, "%02u:%02u", minutes, seconds);
    return SDL_APP_CONTINUE;
}

SDL_AppResult formatTimeControl(TimeControl timeControl, char* output, size_t outputSize) {
    if (!output || outputSize < 11) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter or outputSize less than 11 at " __FILE__);
        return SDL_APP_FAILURE;
    }

    u32 totalSeconds = timeControl.timeLeft / 1000;
    u32 minutesTL = totalSeconds / 60;
    u32 secondsTL = totalSeconds % 60;
    totalSeconds = timeControl.increment / 1000;
    u32 secondsIN = totalSeconds % 60;
    // Format the string as "m:ss + s"
    SDL_snprintf(output, outputSize, "%u:%02u + %u", minutesTL, secondsTL, secondsIN);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderCredits(SDL_FRect rect, App* app) {
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;

    return renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, style.creditsColor, rect);
}
