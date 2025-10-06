#include "../../../../sdl_framework/AppInit.h"

#include "../../../AppUtils.h"

#include "../GameModals.h"

#include "SettingsEvents.h"
#include "SettingsRender.h"

#define SETTING_MODAL_WIDTH_PERCENT (0.75f)
#define SETTING_MODAL_HEIGHT_PERCENT SETTING_MODAL_WIDTH_PERCENT

SDL_FRect calculateSettingsRect(App* app) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(app->state.sdlState.window, &windowWidth, &windowHeight);

    float width = SETTING_MODAL_WIDTH_PERCENT * ((float)windowWidth);
    float height = SETTING_MODAL_HEIGHT_PERCENT * ((float)windowHeight);
    float x = ((float)windowWidth - width) / 2.0;
    float y = ((float)windowHeight - height) / 2.0;
    return (SDL_FRect) { x, y, width, height };
}

SDL_AppResult setSettingsModalActive(App* app) {
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
    SettingsData* modalData = SDL_calloc(1, sizeof(SettingsData));
    SDL_assert(modalData);

    modalData->gameInfo = data->gameInfo;
    modalData->gameInfo.white.engineConfig.enginePath = copyString(data->gameInfo.white.engineConfig.enginePath);
    modalData->gameInfo.black.engineConfig.enginePath = copyString(data->gameInfo.black.engineConfig.enginePath);
    modalData->gameInfo.startingPositionFen = copyString(data->gameInfo.startingPositionFen);
    modalData->currentColor = WHITE;
    modalData->currentStyle = data->appStyle;
    modalData->wasInStartingPositionBefore = true;
    modalData->wasTextInputExited = false;

    // Loading the texture
    const char* settingsImages[2] = { HUMAN_ICON_PATH, COMPUTER_ICON_PATH };
    if (!initializeTextures(&modalData->textures, 2) ||
        !loadImageFromFilePath(&app->state.sdlState, &modalData->textures, settingsImages, 2)) {
        return SDL_APP_FAILURE;
    }

    return setSettingsModalActiveFromCopy(app, modalData);
}

SDL_AppResult setSettingsModalActiveFromCopy(App* app, SettingsData* settingsData) {
    // Initializing the modal
    app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
    app->events.modal.modalRender.renderFunction = &renderSettingsModal;
    app->events.modal.modalRender.onMouseButtonDown = &clickedSettingsModal;
    app->events.modal.modalRender.onMouseHovered = &hoveredSettingsModal;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.data = settingsData;
    app->events.modal.onEscape = &cancelSettingsModal;
    app->events.modal.onReturn = &closedSettingsModal;

    app->events.modal.modalId = SETTINGS_MODAL_ID;
    app->events.modal.isActive = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}
