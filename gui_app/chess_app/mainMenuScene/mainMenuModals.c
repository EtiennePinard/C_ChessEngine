#include "../../../engine/src/utils/CharBuffer.h"

#include "../../sdl_framework/TextInput.h"
#include "../../sdl_framework/CommonEvents.h"
#include "../../sdl_framework/CommonRenderFunctions.h"

#include "../AppStyle.h"
#include "../AppUtils.h"

#include "MainMenuTextInput.h"
#include "MainMenuRender.h"
#include "MainMenuModals.h"

#define MM_MODAL_SIZE_PERCENT (0.5f)
SDL_FRect computeMainMenuModalRect(SDL_Window* window) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    const float modalSize = MM_MODAL_SIZE_PERCENT * SDL_min(windowWidth, windowHeight);
    const float centerX = (float)windowWidth / 2.0;
    const float centerY = (float)windowHeight / 2.0;
    SDL_FRect modalRect = {
        .x = centerX - modalSize / 2,
        .y = centerY - modalSize / 2,
        .w = modalSize,
        .h = modalSize
    };
    return modalRect;
}

#define NUM_TIME_CONTROL_STYLE (3)
#define NUM_TIME_CONTROL_OPTIONS_PER_STYLE (3)
const TimeControl timeControlOptions[NUM_TIME_CONTROL_STYLE][NUM_TIME_CONTROL_OPTIONS_PER_STYLE] = {
{
    {.timeLeft = 60 * 1000, .increment = 0 },
    {.timeLeft = 60 * 1000, .increment = 1000 },
    {.timeLeft = 2 * 60 * 1000, .increment = 1000 },
},
{
    {.timeLeft = 3 * 60 * 1000, .increment = 2000},
    {.timeLeft = 5 * 60 * 1000, .increment = 0 },
    {.timeLeft = 5 * 60 * 1000, .increment = 2000},
},
{
    {.timeLeft = 10 * 60 * 1000, .increment = 0},
    {.timeLeft = 15 * 60 * 1000, .increment = 10000},
    {.timeLeft = 30 * 60 * 1000, .increment = 0},
}
};

SDL_AppResult renderTimeControlModal(SDL_FRect rect, App* app) {
    TimeControlModalData* modalData = (TimeControlModalData*)app->events.modal.data;
    modalData->hovered = (TimeControl){ 0, 0 };

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;
    SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

    // Modal semi-transparent dark background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Modal border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    // Variable title height
    const float titleHeight = rect.h * 0.1;

    const float horizontalPadding = rect.w / (4 * NUM_TIME_CONTROL_OPTIONS_PER_STYLE);
    const float optionWidth = (rect.w - horizontalPadding) / NUM_TIME_CONTROL_OPTIONS_PER_STYLE - horizontalPadding;
    const float optionHeight = optionWidth / 2.0;
    const float verticalEmptySpace = rect.h - titleHeight - optionHeight * NUM_TIME_CONTROL_STYLE;
    const float verticalSeparation = verticalEmptySpace / (NUM_TIME_CONTROL_STYLE + 2);

    SDL_FRect titleRect = {
        .x = rect.x,
        .y = rect.y + verticalSeparation / 2.0,
        .w = rect.w,
        .h = titleHeight
    };
    char* titleText = modalData->playerColor == WHITE ? "Select white's time control" : "Select black's time control";
    SDL_AppResult result = renderSingleLineTextCenteredToFit(renderer, font, titleText, textColor, titleRect);
    if (result != SDL_APP_CONTINUE) return result;

    const float gridTopY = titleRect.y + titleRect.h;
    SDL_FRect optionRect = {
        .x = rect.x + horizontalPadding,
        .y = gridTopY + verticalSeparation,
        .w = optionWidth,
        .h = optionHeight
    };

    for (int styleIndex = 0; styleIndex < NUM_TIME_CONTROL_STYLE; styleIndex++) {
        for (int optionIndex = 0; optionIndex < NUM_TIME_CONTROL_OPTIONS_PER_STYLE; optionIndex++) {
            TimeControl timeControl = timeControlOptions[styleIndex][optionIndex];
            if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &optionRect)) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &optionRect);
                modalData->hovered = timeControl;
            }
            char buffer[11];
            formatTimeControl(timeControl, buffer, 11);
            result = renderSingleLineTextCenteredToFit(renderer, font, buffer, textColor, optionRect);
            if (result != SDL_APP_CONTINUE) return result;
            optionRect.x += optionRect.w + horizontalPadding;
        }
        optionRect.y += optionHeight + verticalSeparation;
        optionRect.x = rect.x + horizontalPadding;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult onTimeControlModalCancel(SDL_Event* event, App* app) {
    (void)event;
    TimeControlModalData* data = (TimeControlModalData*)app->events.modal.data;
    // Resetting the selectedRenderBoxIndex
    if (data->playerColor == WHITE) app->state.currentScene.selectedRenderBoxIndex = WHITE_TIME_CONTROL;
    else if (data->playerColor == BLACK) app->state.currentScene.selectedRenderBoxIndex = BLACK_TIME_CONTROL;
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "The player color of the timeControlModal is %d, which is not white (%d) nor black (%d)\n", data->playerColor, WHITE, BLACK);
        return SDL_APP_FAILURE;
    }
    // Close the modal
    return closeModalEventCallback(event, app);
}

SDL_AppResult clickedTimeControlModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    if (!app->events.modal.isActive || app->events.modal.modalId != TIME_CONTROL_MODAL_ID) return SDL_APP_CONTINUE;

    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    TimeControlModalData* modalData = (TimeControlModalData*)app->events.modal.data;
    // A timeleft of 0 means no time controls were selected
    if (modalData->hovered.timeLeft == 0) return SDL_APP_CONTINUE;

    if (modalData->playerColor == WHITE) {
        data->gameInfo.white.timeControl = modalData->hovered;
        app->state.currentScene.selectedRenderBoxIndex = WHITE_TIME_CONTROL;
    }
    else if (modalData->playerColor == BLACK) {
        data->gameInfo.black.timeControl = modalData->hovered;
        app->state.currentScene.selectedRenderBoxIndex = BLACK_TIME_CONTROL;
    }
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "The player color of the timeControlModal is %d, which is not white (%d) nor black (%d)\n", modalData->playerColor, WHITE, BLACK);
        return SDL_APP_FAILURE;
    }

    // Close the modal
    return closeModalEventCallback(event, app);
}

void setTimeControlModalActive(App* app, PieceCharacteristics colorToSet) {
    // Closing the text input if it is active
    closeTextInput(NULL, app);

    TimeControlModalData* modalData = SDL_malloc(sizeof(TimeControlModalData));
    SDL_assert(modalData);
    modalData->playerColor = colorToSet;
    // Setting the hovered time left to 0 since we did not select any time control yet
    modalData->hovered.timeLeft = 0;

    // Initializing the time control modal
    app->events.modal.modalRender.renderRect = computeMainMenuModalRect(app->state.sdlState.window);
    app->events.modal.modalRender.renderFunction = &renderTimeControlModal;
    app->events.modal.modalRender.onMouseButtonDown = &clickedTimeControlModal;
    app->events.modal.modalRender.onMouseHovered = &rerenderScene;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.data = modalData;
    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.onEscape = &onTimeControlModalCancel;
    app->events.modal.onReturn = NULL;
    app->events.modal.modalId = TIME_CONTROL_MODAL_ID;
    app->events.modal.isActive = true;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}


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

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;
    SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

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
        CHECKBOX_BORDER_COLOR, CHECKBOX_HOVER_COLOR, CHECKBOX_CHECKED_COLOR, CHECKBOX_TEXT_COLOR);
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
        SDL_Color borderColor = BUTTON_BORDER_COLOR;
        SDL_Color textColor = BUTTON_TEXT_COLOR;
        // Border
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderRect(renderer, &timeRect);
        if (!app->events.textInput.isActive || app->events.textInput.textInputId != THINK_TIME_TEXT_INPUT_ID) {
            // Highlight the rectangle if hovered without inputing text
            if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &timeRect)) {
                SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
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
            CHECKBOX_BORDER_COLOR, CHECKBOX_HOVER_COLOR, CHECKBOX_CHECKED_COLOR, CHECKBOX_TEXT_COLOR);
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

    result = renderButton(okRect, app, HOVERING_MODAL, "OK", BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
    if (result != SDL_APP_CONTINUE) return result;

    return renderButton(cancelRect, app, HOVERING_MODAL, "Cancel", BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
}

SDL_AppResult onEngineConfigModalClose(SDL_Event* event, App* app) {
    (void)event;
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;

    if (modalData->wasTextInputExited) {
        // A text input was exited don't close the modal
        modalData->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    // Clicked ok button, saving settings and leaving
    if (modalData->playerColor == WHITE) {
        if (data->gameInfo.white.engineConfig.enginePath) {
            SDL_free(data->gameInfo.white.engineConfig.enginePath);
        }
        data->gameInfo.white.engineConfig = modalData->currentConfig;
        app->state.currentScene.selectedRenderBoxIndex = WHITE_PLAYER_INFO;
    }
    else if (modalData->playerColor == BLACK) {
        if (data->gameInfo.black.engineConfig.enginePath) {
            SDL_free(data->gameInfo.black.engineConfig.enginePath);
        }
        data->gameInfo.black.engineConfig = modalData->currentConfig;
        app->state.currentScene.selectedRenderBoxIndex = BLACK_PLAYER_INFO;
    }
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "The player color of the engineConfigModal is %d, which is not white (%d) nor black (%d)\n", modalData->playerColor, WHITE, BLACK);
        return SDL_APP_FAILURE;
    }

    // Close the modal
    return closeModalEventCallback(event, app);
}

SDL_AppResult onEngineConfigModalCancel(SDL_Event* event, App* app) {
    (void)event;
    EngineConfigModalData* data = (EngineConfigModalData*)app->events.modal.data;

    if (data->wasTextInputExited) {
        // A text input was exited don't cancel the modal
        data->wasTextInputExited = false;
        return SDL_APP_CONTINUE;
    }

    // Resetting the selectedRenderBoxIndex
    if (data->playerColor == WHITE) app->state.currentScene.selectedRenderBoxIndex = WHITE_PLAYER_INFO;
    else if (data->playerColor == BLACK) app->state.currentScene.selectedRenderBoxIndex = BLACK_PLAYER_INFO;
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "The player color of the engineConfigModal is %d, which is not white (%d) nor black (%d)\n", data->playerColor, WHITE, BLACK);
        return SDL_APP_FAILURE;
    }
    if (data->currentConfig.enginePath) SDL_free(data->currentConfig.enginePath);

    // Close the modal
    return closeModalEventCallback(event, app);
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

    // If the modal is not visible or it's not the correct we don't do anything
    if (!app->events.modal.isActive || app->events.modal.modalId != ENGINE_CONFIG_MODAL_ID) return SDL_APP_CONTINUE;

    EngineConfigModalData* modalData = (EngineConfigModalData*)app->events.modal.data;

    SDL_FPoint mousePoint = { event->button.x, event->button.y };
    if (SDL_PointInRectFloat(&mousePoint, &modalData->cancelButtonRect)) {
        // Clicked cancel button, cancelling modal
        if (app->events.textInput.isActive) closeTextInput(event, app);
        app->events.modal.onEscape(event, app);
        app->events.modal.isActive = false;
    }
    else if (SDL_PointInRectFloat(&mousePoint, &modalData->okButtonRect)) {
        // Clicked ok button, closing modal
        if (app->events.textInput.isActive) app->events.textInput.onReturn(event, app);
        if (app->events.textInput.isActive) {
            // The text input is still active, not closing the modal
            modalData->wasTextInputExited = false;
            return SDL_APP_CONTINUE;
        }
        app->events.modal.onReturn(event, app);
        app->events.modal.isActive = false;
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

void setEngineConfigModalActive(App* app, PieceCharacteristics colorToSet) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    EngineConfigModalData* modalData = SDL_malloc(sizeof(EngineConfigModalData));
    SDL_assert(modalData);

    // Closing the text input if it is active
    closeTextInput(NULL, app);

    modalData->playerColor = colorToSet;
    if (colorToSet == WHITE) {
        modalData->currentConfig.isEngine = data->gameInfo.white.engineConfig.isEngine;
        modalData->currentConfig.timeToThink = data->gameInfo.white.engineConfig.timeToThink;
        if (data->gameInfo.white.engineConfig.enginePath) {
            size_t pathLength = SDL_strlen(data->gameInfo.white.engineConfig.enginePath) + 1;
            modalData->currentConfig.enginePath = SDL_calloc(pathLength, sizeof(char));
            SDL_assert(modalData->currentConfig.enginePath);
            SDL_strlcpy(modalData->currentConfig.enginePath, data->gameInfo.white.engineConfig.enginePath, pathLength);
        }
        else {
            modalData->currentConfig.enginePath = NULL;
        }
    }
    else {
        modalData->currentConfig.isEngine = data->gameInfo.black.engineConfig.isEngine;
        modalData->currentConfig.timeToThink = data->gameInfo.black.engineConfig.timeToThink;
        if (data->gameInfo.black.engineConfig.enginePath) {
            size_t pathLength = SDL_strlen(data->gameInfo.black.engineConfig.enginePath) + 1;
            modalData->currentConfig.enginePath = SDL_calloc(pathLength, sizeof(char));
            SDL_assert(modalData->currentConfig.enginePath);
            SDL_strlcpy(modalData->currentConfig.enginePath, data->gameInfo.black.engineConfig.enginePath, pathLength);
        }
        else {
            modalData->currentConfig.enginePath = NULL;
        }
    }

    modalData->wasTextInputExited = false;

    // Initializing the modal
    app->events.modal.modalRender.renderRect = computeMainMenuModalRect(app->state.sdlState.window);
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

    app->events.modal.modalId = ENGINE_CONFIG_MODAL_ID;
    app->events.modal.isActive = true;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}
