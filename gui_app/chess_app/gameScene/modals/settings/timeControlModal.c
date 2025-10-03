#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppUtils.h"
#include "../../../AppStyle.h"

#include "../GameModals.h"

#define NUM_TIME_CONTROL_STYLE (3)
#define NUM_TIME_CONTROL_OPTIONS_PER_STYLE (3)
const TimeControl timeControlOptions2[NUM_TIME_CONTROL_STYLE][NUM_TIME_CONTROL_OPTIONS_PER_STYLE] = {
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

SDL_AppResult renderTimeControlModal2(SDL_FRect rect, App* app) {
    TimeControlModalData2* modalData = (TimeControlModalData2*)app->events.modal.data;
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
    char* titleText = modalData->savedSettingsData->currentColor == WHITE ? "Select white's time control" : "Select black's time control";
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
            TimeControl timeControl = timeControlOptions2[styleIndex][optionIndex];
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

SDL_AppResult onTimeControlModalCancel2(SDL_Event* event, App* app) {
    (void)event;
    TimeControlModalData2* modalData = (TimeControlModalData2*)app->events.modal.data;
    
    SDL_AppResult result = setSettingsModalActiveFromCopy(app, modalData->savedSettingsData);

    SDL_free(modalData);
    return result;
}

SDL_AppResult clickedTimeControlModal2(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    TimeControlModalData2* modalData = (TimeControlModalData2*)app->events.modal.data;
    // A timeleft of 0 means no time controls were selected
    if (modalData->hovered.timeLeft == 0) return SDL_APP_CONTINUE;

    if (modalData->savedSettingsData->currentColor == WHITE) modalData->savedSettingsData->gameInfo.white.timeControl = modalData->hovered;
    else if (modalData->savedSettingsData->currentColor == BLACK) modalData->savedSettingsData->gameInfo.black.timeControl = modalData->hovered;

    // Close the modal
    return onTimeControlModalCancel2(event, app);
}

void setTimeControlModalActive2(App* app) {
    // Closing the text input if it is active
    closeTextInput(NULL, app);

    // We can only activate the time control modal from the settings modal
    SDL_assert(app->events.modal.modalId == SETTINGS_MODAL_ID);
    SettingsData* settingsData = (SettingsData*)app->events.modal.data;

    TimeControlModalData2* modalData = SDL_malloc(sizeof(TimeControlModalData2));
    SDL_assert(modalData);
    // Setting the hovered time left to 0 since we did not select any time control yet
    modalData->hovered.timeLeft = 0;
    modalData->savedSettingsData = settingsData;

    // Initializing the time control modal
    app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
    app->events.modal.modalRender.renderFunction = &renderTimeControlModal2;
    app->events.modal.modalRender.onMouseButtonDown = &clickedTimeControlModal2;
    app->events.modal.modalRender.onMouseHovered = &rerenderScene;
    app->events.modal.modalRender.onMouseButtonUp = NULL;
    app->events.modal.modalRender.onMouseWheelScrolled = NULL;
    app->events.modal.modalRender.onMouseEntered = NULL;
    app->events.modal.modalRender.onMouseExited = NULL;

    app->events.modal.data = modalData;
    app->events.modal.canOnlyInteractWithModal = true;
    app->events.modal.onEscape = &onTimeControlModalCancel2;
    app->events.modal.onReturn = NULL;
    app->events.modal.modalId = TIME_CONTROL_MODAL_ID2;
    app->events.modal.isActive = true;

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
}