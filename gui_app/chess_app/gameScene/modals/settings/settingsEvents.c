#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/AppCleanup.h"
#include "../../../../sdl_framework/TextInputEvents.h"

#include "../../GameEvents.h"

#include "SettingsTextInput.h"
#include "SettingsEvents.h"

SDL_AppResult closedSettingsModal(SDL_Event* event, App* app) {
    (void)event;

    SettingsData* modalData = (SettingsData*)app->events.modal.data;
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

    if (modalData->wasTextInputExited) {
        // A text input was exited don't close the modal
        modalData->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    // Clicked ok button, saving settings and leaving
    if (data->gameInfo.white.engineConfig.enginePath) SDL_free(data->gameInfo.white.engineConfig.enginePath);
    if (data->gameInfo.black.engineConfig.enginePath) SDL_free(data->gameInfo.black.engineConfig.enginePath);
    if (data->gameInfo.startingPositionFen) SDL_free(data->gameInfo.startingPositionFen);
    data->gameInfo = modalData->gameInfo;

    // Resetting the game to the new settings
    LoadGameInfoResult loadGameInfoResult = loadGameInfo(app);

    size_t messageSize;
    char* message;
    switch (loadGameInfoResult) {
    case INVALID_FEN_STRING:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when loading the position\n");
        return SDL_APP_FAILURE;

    case EMPTY_WHITE_ENGINE_PATH:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Empty white engine path", "The white engine path is not set", app->state.sdlState.window);
        return SDL_APP_CONTINUE;

    case INVALID_WHITE_ENGINE_PATH:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for white at path `%s`\n", data->gameInfo.white.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", data->gameInfo.white.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", data->gameInfo.white.engineConfig.enginePath);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid white engine path", message, app->state.sdlState.window);
        SDL_free(message);
        return SDL_APP_CONTINUE;

    case EMPTY_BLACK_ENGINE_PATH:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Empty black engine path", "The black engine path is not set", app->state.sdlState.window);
        return SDL_APP_CONTINUE;

    case INVALID_BLACK_ENGINE_PATH:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize the engine for black at path `%s`\n", data->gameInfo.black.engineConfig.enginePath);
        messageSize = SDL_snprintf(NULL, 0, "The engine path %s is invalid", data->gameInfo.black.engineConfig.enginePath) + 1;
        message = SDL_calloc(messageSize, sizeof(char));
        SDL_snprintf(message, messageSize, "The engine path %s is invalid", data->gameInfo.black.engineConfig.enginePath);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid black engine path", message, app->state.sdlState.window);
        SDL_free(message);
        return SDL_APP_CONTINUE;

    case SUCCESS:
    default: break;
    }

    cleanupTextures(modalData->textures);
    SDL_free(modalData->textures.data);

    resetGame(app);

    return closeModalEventCallback(event, app);
}

SDL_AppResult cancelSettingsModal(SDL_Event* event, App* app) {
    (void)event;

    SettingsData* modalData = (SettingsData*)app->events.modal.data;

    if (modalData->wasTextInputExited) {
        // A text input was exited don't close the modal
        modalData->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    // Clicked ok button, saving settings and leaving
    SDL_free(modalData->gameInfo.white.engineConfig.enginePath);
    SDL_free(modalData->gameInfo.black.engineConfig.enginePath);
    SDL_free(modalData->gameInfo.startingPositionFen);

    cleanupTextures(modalData->textures);
    SDL_free(modalData->textures.data);

    return closeModalEventCallback(event, app);
}

SDL_AppResult clickedSettingsModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    SettingsData* data = (SettingsData*)app->events.modal.data;
    if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->cancelButton)) {
        return cancelSettingsModal(event, app);
    }
    else if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->saveButton)) {
        return closedSettingsModal(event, app);
    }
    else if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->colorToggle)) {
        // Switch the color
        data->currentColor = data->currentColor == WHITE ? BLACK : WHITE;
    }
    else if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->engineConfig)) {
        setEngineConfigModalActive2(app);
    }
    else if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->timeControl)) {
        setTimeControlModalActive2(app);
    }
    else if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->startingPosition)) {
        if (!app->events.textInput.isActive) {
            setStartingPositionTextInputActive_2(data->startingPosition, app);
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult hoveredSettingsModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    SettingsData* data = (SettingsData*)app->events.modal.data;
    if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->startingPosition)) {
        if (!data->wasInStartingPositionBefore) {
            data->wasInStartingPositionBefore = true;
            return changeMouseIconOnEnterTextInput(event, rect, app);
        }
    }
    else if (data->wasInStartingPositionBefore) {
        data->wasInStartingPositionBefore = false;
        return resetMouseIconOnExitTextInput(event, rect, app);
    }
    return SDL_APP_CONTINUE;
}
