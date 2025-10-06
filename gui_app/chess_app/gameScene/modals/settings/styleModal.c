#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppUtils.h"
#include "../../../AppStyle.h"

#include "../GameModals.h"

typedef struct StringKeyAppStyleValue {
    char* appStyleName;
    AppStyle* styleValue;
} StringKeyAppStyleValue;

#define STYLE_ROW (1)
#define STYLE_COLUMN (1)
const StringKeyAppStyleValue stringKeyStyleValueMap[STYLE_ROW][STYLE_COLUMN] = {
{
    {.appStyleName = "default style", .styleValue = &defaultStyle },
},
};

SDL_AppResult renderStyleModal(SDL_FRect rect, App* app) {
    StyleModalData* modalData = (StyleModalData*)app->events.modal.data;
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;

    modalData->isStyleHovered = false;

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = style.buttonStyle.borderColor;
    SDL_Color textColor = style.textStyle.textColor;
    SDL_Color highlightColor = style.buttonStyle.hoverColor;
    SDL_Color backgroundColor = style.backgroundColor;

    // Modal semi-transparent dark background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Modal border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    // Variable title height
    const float titleHeight = rect.h * 0.1;

    const float horizontalPadding = rect.w / (4 * STYLE_COLUMN);
    const float optionWidth = (rect.w - horizontalPadding) / STYLE_COLUMN - horizontalPadding;
    const float optionHeight = optionWidth / 2.0;
    const float verticalEmptySpace = rect.h - titleHeight - optionHeight * STYLE_ROW;
    const float verticalSeparation = verticalEmptySpace / (STYLE_ROW + 2);

    SDL_FRect titleRect = {
        .x = rect.x,
        .y = rect.y + verticalSeparation / 2.0,
        .w = rect.w,
        .h = titleHeight
    };
    const char* titleText = "Select app's style";
    SDL_AppResult result = renderSingleLineTextCenteredToFit(renderer, font, titleText, textColor, titleRect);
    if (result != SDL_APP_CONTINUE) return result;

    const float gridTopY = titleRect.y + titleRect.h;
    SDL_FRect optionRect = {
        .x = rect.x + horizontalPadding,
        .y = gridTopY + verticalSeparation,
        .w = optionWidth,
        .h = optionHeight
    };

    for (int styleIndex = 0; styleIndex < STYLE_ROW; styleIndex++) {
        for (int optionIndex = 0; optionIndex < STYLE_COLUMN; optionIndex++) {
            StringKeyAppStyleValue keyValue = stringKeyStyleValueMap[styleIndex][optionIndex];
            if (SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &optionRect)) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &optionRect);
                modalData->hovered = keyValue.styleValue;
                modalData->isStyleHovered = true;
            }
            result = renderSingleLineTextCenteredToFit(renderer, font, keyValue.appStyleName, textColor, optionRect);
            if (result != SDL_APP_CONTINUE) return result;
            optionRect.x += optionRect.w + horizontalPadding;
        }
        optionRect.y += optionHeight + verticalSeparation;
        optionRect.x = rect.x + horizontalPadding;
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

SDL_AppResult clickedStyleModal(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    StyleModalData* modalData = (StyleModalData*)app->events.modal.data;
    if (!modalData->isStyleHovered) return SDL_APP_CONTINUE;

    modalData->savedSettingsData->currentStyle = *modalData->hovered;

    // Close the modal
    return onStyleModalCancel(event, app);
}

void setStyleModalActive(App* app) {
    // Closing the text input if it is active
    closeTextInput(NULL, app);

    // We can only activate the time control modal from the settings modal
    SDL_assert(app->events.modal.modalId == SETTINGS_MODAL_ID);
    SettingsData* settingsData = (SettingsData*)app->events.modal.data;

    StyleModalData* modalData = SDL_malloc(sizeof(StyleModalData));
    SDL_assert(modalData);
    // Setting the hovered time left to 0 since we did not select any time control yet
    modalData->hovered->backgroundColor.a = 69;
    modalData->isStyleHovered = false;
    modalData->savedSettingsData = settingsData;

    // Initializing the time control modal
    app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
    app->events.modal.modalRender.renderFunction = &renderStyleModal;
    app->events.modal.modalRender.onMouseButtonDown = &clickedStyleModal;
    app->events.modal.modalRender.onMouseHovered = &rerenderScene;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.data = modalData;
    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.onEscape = &onStyleModalCancel;
    app->events.modal.onReturn = NULL;
    app->events.modal.modalId = TIME_CONTROL_MODAL_ID;
    app->events.modal.isActive = true;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}
