#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppStyle.h"
#include "../../../AppUtils.h"

#include "../GameModals.h"

#include "SettingsTextInput.h"

SDL_AppResult renderEnginePathButton(SDL_FRect rect, App* app, SDL_Color highlightColor, SDL_Color textColor) {
    EngineConfigModalData* data = (EngineConfigModalData*)app->events.modal.data;
    char* label = data->currentConfig.enginePath ? data->currentConfig.enginePath : "Set engine's path";

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    // If this is the currently hovered box, highlight it
    if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &rect)) {
        SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    // Render button text
    return renderSingleLineTextCenteredToFit(renderer, font, label, textColor, rect);
}

SDL_AppResult renderEngineConfigModal(SDL_FRect rect, App* app) {
    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = style.buttonStyle.borderColor;
    SDL_Color textColor = style.textStyle.textColor;
    SDL_Color highlightColor = style.buttonStyle.hoverColor;
    SDL_Color backgroundColor = style.backgroundColor;

    const float padding = rect.h * 0.05f;
    const float lineHeight = TTF_GetFontLineSkip(font);
    const float buttonHeight = lineHeight * 1.5f;

    // Modal background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Modal border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    float y = rect.y + padding;

    // Modal Title
    SDL_FRect titleRect = {
        .x = rect.x + padding,
        .y = y,
        .w = rect.w - 2 * padding,
        .h = lineHeight
    };
    SDL_AppResult result = renderSingleLineTextCenteredToFit(renderer, font, "Engine Configuration", textColor, titleRect);
    if (result != SDL_APP_CONTINUE) return result;

    y += lineHeight + padding;

    // Checkbox: Use Engine
    SDL_FRect checkboxRect = {
        .x = rect.x + padding,
        .y = y,
        .w = rect.w - 2 * padding,
        .h = buttonHeight
    };
    modalData->checkboxRect = checkboxRect;
    result = renderLabeledCheckboxButton(checkboxRect, app,
        modalData->currentConfig.isEngine, "Use Engine", HOVERING_MODAL,
        style.checkboxStyle.borderColor,
        style.checkboxStyle.hoverColor,
        style.checkboxStyle.checkedColor,
        style.textStyle.textColor);
    if (result != SDL_APP_CONTINUE) return result;

    y += buttonHeight + padding;

    // Engine Path and Think Time if isEngine enabled
    if (modalData->currentConfig.isEngine) {
        // Engine Path Button
        SDL_FRect pathRect = {
            .x = rect.x + padding,
            .y = y,
            .w = rect.w - 2 * padding,
            .h = buttonHeight
        };
        modalData->enginePathRect = pathRect;
        result = renderEnginePathButton(pathRect, app, highlightColor, textColor);
        if (result != SDL_APP_CONTINUE) return result;

        y += buttonHeight + padding;

        // Think Time Button
        SDL_FRect timeRect = {
            .x = rect.x + padding,
            .y = y,
            .w = rect.w - 2 * padding,
            .h = buttonHeight
        };
        modalData->thinkTimeRect = timeRect;

        // Border
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderRect(renderer, &timeRect);
        if (!app->events.textInput.isActive || app->events.textInput.textInputId != THINK_TIME_TEXT_INPUT_ID) {
            // Highlight the rectangle if hovered without inputing text
            if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &timeRect)) {
                SDL_Color highlightColor = style.buttonStyle.hoverColor;
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &timeRect);
            }

            if (modalData->currentConfig.timeToThink == 0) {
                result = renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font,
                    "Engine thinks by himself", textColor, timeRect);
            }
            else {
                // If there already is a time to think set it
                int size = SDL_snprintf(NULL, 0, "%u ms", modalData->currentConfig.timeToThink) + 1;
                char text[size];
                SDL_snprintf(text, size, "%u ms", modalData->currentConfig.timeToThink);
                result = renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, text, textColor, timeRect);
            }
        }

        // This is for updating the rectangle when resizing the screen
        if (app->events.textInput.isActive && app->events.textInput.textInputId == THINK_TIME_TEXT_INPUT_ID) {
            app->events.textInput.textInputRender.renderRect = timeRect;
        }

        y += buttonHeight + padding;

        SDL_FRect thinksByHimselfRect = {
            .x = rect.x + padding,
            .y = y,
            .w = rect.w - 2 * padding,
            .h = buttonHeight
        };
        result = renderLabeledCheckboxButton(thinksByHimselfRect, app,
            modalData->currentConfig.timeToThink == 0, "Engine thinks on its own", HOVERING_MODAL,
            style.checkboxStyle.borderColor,
            style.checkboxStyle.hoverColor,
            style.checkboxStyle.checkedColor,
            style.textStyle.textColor);
        modalData->thinkByHimselfRect = thinksByHimselfRect;
        if (result != SDL_APP_CONTINUE) return result;
    }

    // OK and Cancel buttons
    const float buttonSpacing = padding;
    const float buttonWidth = (rect.w - 3 * padding) / 2.0f;

    SDL_FRect okRect = {
        .x = rect.x + padding,
        .y = rect.y + rect.h - buttonHeight - padding,
        .w = buttonWidth,
        .h = buttonHeight
    };
    SDL_FRect cancelRect = {
        .x = okRect.x + okRect.w + buttonSpacing,
        .y = okRect.y,
        .w = buttonWidth,
        .h = buttonHeight
    };

    modalData->okButtonRect = okRect;
    modalData->cancelButtonRect = cancelRect;

    result = renderButton(okRect, app, HOVERING_MODAL, "OK",
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor);
    if (result != SDL_APP_CONTINUE) return result;

    return renderButton(cancelRect, app, HOVERING_MODAL, "Cancel",
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor);
}

SDL_AppResult onEngineConfigModalClose(SDL_Event* event, App* app) {
    (void)event;
    EngineConfigModalData* data = (EngineConfigModalData*)app->events.modal.data;

    if (data->wasTextInputExited) {
        // A text input was exited don't close the modal
        data->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    // Clicked ok button, saving settings and leaving
    if (data->savedSettingsData->currentColor == WHITE) {
        if (data->savedSettingsData->gameInfo.white.engineConfig.enginePath) {
            SDL_free(data->savedSettingsData->gameInfo.white.engineConfig.enginePath);
        }
        data->savedSettingsData->gameInfo.white.engineConfig = data->currentConfig;
    }
    else if (data->savedSettingsData->currentColor == BLACK) {
        if (data->savedSettingsData->gameInfo.black.engineConfig.enginePath) {
            SDL_free(data->savedSettingsData->gameInfo.black.engineConfig.enginePath);
        }
        data->savedSettingsData->gameInfo.black.engineConfig = data->currentConfig;
    }

    SDL_AppResult result = setSettingsModalActiveFromCopy(app, data->savedSettingsData);

    SDL_free(data);
    return result;
}

SDL_AppResult onEngineConfigModalCancel(SDL_Event* event, App* app) {
    (void)event;
    EngineConfigModalData* data = (EngineConfigModalData*)app->events.modal.data;

    if (data->wasTextInputExited) {
        // A text input was exited don't cancel the modal
        data->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    if (data->currentConfig.enginePath) SDL_free(data->currentConfig.enginePath);

    SDL_AppResult result = setSettingsModalActiveFromCopy(app, data->savedSettingsData);

    SDL_free(data);
    return result;
}

static void onEnginePathSelected(void* userdata, const char* const* filelist, int filterIndex) {
    (void)filterIndex;

    App* app = (App*)userdata;
    // Re-enabling events and rendering
    app->events.shouldHandleEvents = true;

    // Handle error
    if (filelist == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "File dialog error: %s", SDL_GetError());
        return;
    }

    // Handle cancel
    if (filelist[0] == NULL) {
        SDL_Log("File dialog canceled.");
        return;
    }

    // Use the first selected file
    size_t length = SDL_strlen(filelist[0]) + 1;
    char* enginePath = SDL_calloc(length, sizeof(char));
    SDL_strlcpy(enginePath, filelist[0], length);

    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

    // Saving the enginePath to the current config
    if (modalData->currentConfig.enginePath != NULL) SDL_free(modalData->currentConfig.enginePath);
    modalData->currentConfig.enginePath = enginePath;
}

SDL_AppResult clickedEnginePath(App* app) {
    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;
    if (!modalData->currentConfig.isEngine) return SDL_APP_CONTINUE;

    SDL_DialogFileFilter filters[] = { { "All Files", "*" } };

    const char* path = SDL_GetBasePath();
    if (!path) return SDL_APP_FAILURE;
    SDL_ShowOpenFileDialog(
        &onEnginePathSelected,
        app,
        app->state.sdlState.window,
        filters, 1,
        path,
        false
    );

    // Disabling events and rendering
    app->events.shouldHandleEvents = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, NO_RERENDER);

    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedThinkTime(SDL_FRect rect, App* app) {
    // If the text input is already active, do nothing
    if (app->events.textInput.isActive) return SDL_APP_CONTINUE;

    setTimeToThinkTextInputActive(rect, app);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedEngineConfigModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;

    SDL_FPoint mousePoint = { event->button.x, event->button.y };
    if (SDL_PointInRectFloat(&mousePoint, &modalData->cancelButtonRect)) {
        // Clicked cancel button, cancelling modal
        if (app->events.textInput.isActive) closeTextInput(event, app);
        return app->events.modal.onEscape(event, app);
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->okButtonRect)) {
        // Clicked ok button, closing modal
        if (app->events.textInput.isActive) app->events.textInput.onReturn(event, app);
        if (app->events.textInput.isActive) {
            // The text input is still active, not closing the modal
            modalData->wasTextInputExited = false;
            return SDL_APP_CONTINUE;
        }
        return app->events.modal.onReturn(event, app);
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->checkboxRect)) {
        if (app->events.textInput.isActive) closeTextInput(event, app);

        // Switching isEngine to its opposite value
        modalData->currentConfig.isEngine = !modalData->currentConfig.isEngine;
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->enginePathRect) &&
        modalData->currentConfig.isEngine) {
        return clickedEnginePath(app);
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->thinkTimeRect) &&
        modalData->currentConfig.isEngine) {
        // If the think time is 0 then set it to the default value
        if (modalData->currentConfig.timeToThink == 0) {
            modalData->currentConfig.timeToThink = DEFAULT_TIME_TO_THINK;
        }
        return clickedThinkTime(modalData->thinkTimeRect, app);
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->thinkByHimselfRect) &&
        modalData->currentConfig.isEngine) {
        if (modalData->currentConfig.timeToThink == 0) {
            modalData->currentConfig.timeToThink = DEFAULT_TIME_TO_THINK;
            clickedThinkTime(modalData->thinkTimeRect, app);
        }
        else {
            // Put think time to 0
            modalData->currentConfig.timeToThink = 0;
        }
    }

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

void setEngineConfigModalActive(App* app) {
    SettingsData* data = (SettingsData*)app->events.modal.data;
    EngineConfigModalData* modalData = SDL_malloc(sizeof(EngineConfigModalData));
    SDL_assert(modalData);

    // Closing the text input if it is active
    closeTextInput(NULL, app);

    if (data->currentColor == WHITE) {
        modalData->currentConfig.isEngine = data->gameInfo.white.engineConfig.isEngine;
        modalData->currentConfig.timeToThink = data->gameInfo.white.engineConfig.timeToThink;
        if (data->gameInfo.white.engineConfig.enginePath) modalData->currentConfig.enginePath = copyString(data->gameInfo.white.engineConfig.enginePath);
        else modalData->currentConfig.enginePath = NULL;
    }
    else {
        modalData->currentConfig.isEngine = data->gameInfo.black.engineConfig.isEngine;
        modalData->currentConfig.timeToThink = data->gameInfo.black.engineConfig.timeToThink;
        if (data->gameInfo.black.engineConfig.enginePath) modalData->currentConfig.enginePath = copyString(data->gameInfo.black.engineConfig.enginePath);
        else modalData->currentConfig.enginePath = NULL;
    }

    modalData->wasTextInputExited = false;
    modalData->savedSettingsData = data;

    // Initializing the modal
    app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
    app->events.modal.modalRender.renderFunction = &renderEngineConfigModal;
    app->events.modal.modalRender.onMouseButtonDown = &clickedEngineConfigModal;
    app->events.modal.modalRender.onMouseHovered = &rerenderScene;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.data = modalData;
    app->events.modal.onEscape = &onEngineConfigModalCancel;
    app->events.modal.onReturn = &onEngineConfigModalClose;

    app->events.modal.modalId = ENGINE_CONFIG_MODAL_ID2;
    app->events.modal.isActive = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}
