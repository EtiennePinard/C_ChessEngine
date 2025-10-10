#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppUtils.h"
#include "../../../AppStyle.h"

#include "../GameModals.h"

typedef struct Buttons {
    ButtonStyle buttonStyle;
    SDL_Color textColor;

    size_t numButton;
    char** titles;
} Buttons;

#define VERTICAL_SEPARATION_PERCENT (0.3)

SDL_AppResult renderButtonGrid(SDL_FRect rectToFit, App* app,
    int numRows, int numItemsPerRow[numRows],
    Buttons buttons, SDL_FRect* buttonRects) {

    const float verticalSeparation = rectToFit.h / numRows * VERTICAL_SEPARATION_PERCENT;
    const float optionHeight = rectToFit.h / numRows - verticalSeparation;

    SDL_FRect buttonRect = {
       .y = rectToFit.y,
       .h = optionHeight
    };
    size_t buttonIndex = 0;
    for (int row = 0; row < numRows; row++) {
        int numElementsInRow = numItemsPerRow[row];
        const float horizontalPadding = rectToFit.w / numElementsInRow / 4.0;
        buttonRect.w = (rectToFit.w - horizontalPadding) / numElementsInRow - horizontalPadding;
        buttonRect.x = rectToFit.x + horizontalPadding;
        for (int colIndex = 0; colIndex < numElementsInRow; colIndex++) {
            char* buttonTitle = buttons.titles[buttonIndex];
            if (renderButton(
                buttonRect, app,
                HOVERING_MODAL, buttonTitle,
                buttons.buttonStyle.hoverColor,
                buttons.buttonStyle.clickedColor,
                buttons.buttonStyle.idleColor,
                buttons.buttonStyle.borderColor,
                buttons.textColor) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
            buttonRects[buttonIndex] = buttonRect;
            buttonIndex++;
            SDL_assert(buttonIndex <= buttons.numButton);

            buttonRect.x += buttonRect.w + horizontalPadding;
        }
        buttonRect.y += buttonRect.h + verticalSeparation;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderStyleModal(SDL_FRect rect, App* app) {
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;
    StyleModalData* modalData = (StyleModalData*)app->events.modal.data;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = style.buttonStyle.borderColor;
    SDL_Color textColor = style.textStyle.textColor;
    SDL_Color backgroundColor = style.backgroundColor;

    // Modal semi-transparent dark background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Modal border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    // Variable title height
    const float titleHeight = rect.h * 0.1;

    const float verticalSeparation = titleHeight;

    SDL_FRect titleRect = {
        .x = rect.x,
        .y = rect.y + verticalSeparation / 2.0,
        .w = rect.w,
        .h = titleHeight
    };
    const char* titleText = "Select app's style";
    if (renderSingleLineTextCenteredToFit(renderer, font,
        titleText, textColor, titleRect) != SDL_APP_CONTINUE)
        return SDL_APP_FAILURE;

    const float gridTopY = titleRect.y + titleRect.h + verticalSeparation;
    SDL_FRect buttonGridRect = {
        .x = rect.x,
        .y = gridTopY,
        .w = rect.w,
        .h = rect.h - 2 * verticalSeparation - titleRect.h
    };

    Buttons buttons = {
        .buttonStyle = style.buttonStyle,
        .textColor = style.textStyle.textColor,
        .numButton = NUM_STYLES,
        .titles = modalData->buttonTitles
    };

    renderButtonGrid(buttonGridRect, app, 2, (int[]) { 2, 3 }, buttons, modalData->buttonRects);

    return SDL_APP_CONTINUE;
}

SDL_AppResult onButtonClick(App* app, AppStyle* appStyle) {
    GameSceneData* sceneData = (GameSceneData*)app->state.currentScene.data;
    // We want to immediately change the style and rerender
    // so the user can see if they like the new style or not
    // and then change back if they did not like it
    sceneData->appStyle = *appStyle;
    // We also want to change the background color
    app->state.currentScene.sceneRender.renderDrawColor = sceneData->appStyle.backgroundColor;
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickedDownStyleModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    StyleModalData* modalData = (StyleModalData*)app->events.modal.data;
    for (size_t buttonIndex = 0; buttonIndex < NUM_STYLES; buttonIndex++) {
        if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &modalData->buttonRects[buttonIndex])) {
            // We assume we cannot be in two buttons at once
            return onButtonClick(app, modalData->appStyles[buttonIndex]);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult onStyleModalCancel(SDL_Event* event, App* app) {
    (void)event;
    StyleModalData* modalData = (StyleModalData*)app->events.modal.data;

    SDL_AppResult result = setSettingsModalActiveFromCopy(app, modalData->savedSettingsData);

    SDL_free(modalData);
    return result;
}

void setStyleModalActive(App* app) {
    // Closing the text input if it is active
    closeTextInput(NULL, app);

    // We can only activate the time control modal from the settings modal
    SDL_assert(app->events.modal.modalId == SETTINGS_MODAL_ID);
    SettingsData* settingsData = (SettingsData*)app->events.modal.data;

    StyleModalData* modalData = SDL_malloc(sizeof(StyleModalData));
    SDL_assert(modalData);
    modalData->savedSettingsData = settingsData;
    char* titles[NUM_STYLES] = { "Default Gray", "Midnight Blue", "Forest Green", "Solarized Light", "Royal Purple" };
    AppStyle* appStyles[NUM_STYLES] = { &defaultStyle, &midnightBlueStyle, &forestGreenStyle, &solarizedLightStyle, &royalPurpleStyle };
    memcpy(&modalData->buttonTitles, titles, sizeof(titles));
    memcpy(&modalData->appStyles, appStyles, sizeof(appStyles));

    // Initializing the time control modal
    app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
    app->events.modal.modalRender.renderFunction = &renderStyleModal;
    app->events.modal.modalRender.onMouseHovered = &rerenderScene; // So that we can detect hover behaviour
    app->events.modal.modalRender.onMouseButtonDown = &clickedDownStyleModal;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.data = modalData;
    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.onEscape = &onStyleModalCancel;
    app->events.modal.onReturn = NULL;
    app->events.modal.modalId = STYLE_MODAL_ID;
    app->events.modal.isActive = true;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}
