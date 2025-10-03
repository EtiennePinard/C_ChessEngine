#include "../../../../../engine/src/utils/FenString.h"

#include "../../../../sdl_framework/CommonEvents.h"
#include "../../../../sdl_framework/TextInputEvents.h"
#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppStyle.h"

#include "../GameModals.h"
#include "SettingsTextInput.h"

SDL_AppResult startingPositionTextInputReturn_2(SDL_Event* event, App* app) {
    (void)event;
    SettingsData* data = (SettingsData*)app->events.modal.data;
    data->wasTextInputExited = true;

    ChessPosition dummyPosition;
    char fenCopy[app->events.textInput.text.count + 1];
    SDL_strlcpy(fenCopy, app->events.textInput.text.data, app->events.textInput.text.count + 1);
    if (!FenString_setChessPositionFromFenString(fenCopy, &dummyPosition)) {
        SDL_Log("The inputted fen string, `%s`, is incorrect\n", app->events.textInput.text.data);
        int messageSize = SDL_snprintf(NULL, 0, "The inputted fen string, `%s`, is not a valid fen string", app->events.textInput.text.data) + 1;
        char message[messageSize];
        SDL_snprintf(message, messageSize, "The inputted fen string, `%s` is not a valid fen string", app->events.textInput.text.data);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fen string error", message, app->state.sdlState.window);
    }
    else {
        // The fen string is correct, store it into the gameInfo field
        if (data->gameInfo.startingPositionFen) SDL_free(data->gameInfo.startingPositionFen);
        data->gameInfo.startingPositionFen = SDL_calloc(app->events.textInput.text.count + 1, sizeof(char));
        SDL_assert(data->gameInfo.startingPositionFen);
        SDL_strlcpy(data->gameInfo.startingPositionFen, app->events.textInput.text.data, app->events.textInput.text.count + 1);
        return closeTextInput(event, app);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult startingPositionTextInputCancel(SDL_Event* event, App* app) {
    SettingsData* modalData = (SettingsData*)app->events.modal.data;
    modalData->wasTextInputExited = true;
    return closeTextInput(event, app);
}

SDL_AppResult renderStartingPosition_2(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color borderColor = BUTTON_BORDER_COLOR;

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    return renderTextInputCenteredToFit(rect, app,
        BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR, SELECTED_TEXT_COLOR, SELECTED_TEXT_BG_COLOR);
}

void setStartingPositionTextInputActive_2(SDL_FRect rect, App* app) {
    app->events.textInput.textInputRender.renderRect = rect;
    app->events.textInput.textInputRender.renderFunction = &renderStartingPosition_2;
    app->events.textInput.textInputRender.onMouseButtonDown = &resetTextInputSelectionOnMouseButtonDown;
    app->events.textInput.textInputRender.onMouseButtonUp = NULL;
    app->events.textInput.textInputRender.onMouseEntered = &changeMouseIconOnEnterTextInput;
    app->events.textInput.textInputRender.onMouseExited = &resetMouseIconOnExitTextInput;
    app->events.textInput.textInputRender.onMouseHovered = &updateTextInputSelectionOnMouseHovered;
    app->events.textInput.textInputRender.onMouseWheelScrolled = NULL;

    SDL_Rect area = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
    app->events.textInput.text.capacity = MAX_FEN_STRING_SIZE;
    app->events.textInput.text.count = 0;
    // we assume that text.data is NULL
    SDL_assert(!app->events.textInput.text.data);
    app->events.textInput.text.data = SDL_calloc(app->events.textInput.text.capacity, sizeof(char));
    SDL_assert(app->events.textInput.text.data);

    app->events.textInput.onEscape = &startingPositionTextInputCancel;
    app->events.textInput.onReturn = &startingPositionTextInputReturn_2;
    app->events.textInput.onKeyDown = &textInputKeyDown;
    app->events.textInput.onTextInputEvent = &appendTextToTextInputOnTextInputEvent;

    app->events.textInput.keepOnlyAscii = true;

    app->events.textInput.textInputId = STARTING_POSITION_TEXT_INPUT_ID;
    app->events.textInput.isActive = true;

    app->events.textInput.cursor.index = 0;
    app->events.textInput.cursor.sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    SDL_SetCursor(app->events.textInput.cursor.sdlCursor);

    SDL_SetTextInputArea(app->state.sdlState.window,
        &area,
        area.w / 2);
    SDL_StartTextInput(app->state.sdlState.window);
}


SDL_AppResult thinkTimeTextInputReturn2(SDL_Event* event, App* app) {
    (void)event;
    int parsedThinkTime = string_parseNumber(app->events.textInput.text.data);

    EngineConfigModalData2* modalData = (EngineConfigModalData2*)app->events.modal.data;
    modalData->wasTextInputExited = true;

    if (parsedThinkTime == -1) {
        SDL_Log("The inputted think time, `%s`, is incorrect\n", app->events.textInput.text.data);
        int messageSize = SDL_snprintf(NULL, 0, "The inputted think time, `%s`, is not a valid think time", app->events.textInput.text.data) + 1;
        char message[messageSize];
        SDL_snprintf(message, messageSize, "The inputted think time, `%s`, is not a valid think time", app->events.textInput.text.data);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Think time error", message, app->state.sdlState.window);
    }
    else {
        // The think time is correct, store it into the currentConfig field
        modalData->currentConfig.timeToThink = parsedThinkTime;
        closeTextInput(event, app);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult thinkTimeTextInputCancel2(SDL_Event* event, App* app) {
    EngineConfigModalData2* modalData = (EngineConfigModalData2*)app->events.modal.data;
    modalData->wasTextInputExited = true;
    return closeTextInput(event, app);
}

SDL_AppResult renderThinkTime2(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color borderColor = BUTTON_BORDER_COLOR;

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    return renderTextInputCenteredToFit(rect, app,
        BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR, SELECTED_TEXT_COLOR, SELECTED_TEXT_BG_COLOR);
}

void setTimeToThinkTextInputActive_2(SDL_FRect rect, App* app) {
    app->events.textInput.textInputRender.renderRect = rect;
    app->events.textInput.textInputRender.renderFunction = &renderThinkTime2;
    app->events.textInput.textInputRender.onMouseButtonDown = &resetTextInputSelectionOnMouseButtonDown;
    app->events.textInput.textInputRender.onMouseButtonUp = NULL;
    app->events.textInput.textInputRender.onMouseEntered = &changeMouseIconOnEnterTextInput;
    app->events.textInput.textInputRender.onMouseExited = &resetMouseIconOnExitTextInput;
    app->events.textInput.textInputRender.onMouseHovered = &updateTextInputSelectionOnMouseHovered;
    app->events.textInput.textInputRender.onMouseWheelScrolled = NULL;

    SDL_Rect area = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
    app->events.textInput.text.capacity = MAX_FEN_STRING_SIZE;
    app->events.textInput.text.count = 0;
    // we assume that text.data is NULL
    SDL_assert(!app->events.textInput.text.data);
    app->events.textInput.text.data = SDL_calloc(app->events.textInput.text.capacity, sizeof(char));
    SDL_assert(app->events.textInput.text.data);

    app->events.textInput.onEscape = &thinkTimeTextInputCancel2;
    app->events.textInput.onReturn = &thinkTimeTextInputReturn2;
    app->events.textInput.onKeyDown = &textInputKeyDown;
    app->events.textInput.onTextInputEvent = &appendTextToTextInputOnTextInputEvent;

    app->events.textInput.keepOnlyAscii = true;
    app->events.textInput.textInputId = THINK_TIME_TEXT_INPUT_ID;
    app->events.textInput.isActive = true;

    app->events.textInput.cursor.index = 0;
    app->events.textInput.cursor.sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    SDL_SetCursor(app->events.textInput.cursor.sdlCursor);

    SDL_SetTextInputArea(app->state.sdlState.window,
        &area,
        area.w / 2);
    SDL_StartTextInput(app->state.sdlState.window);
}
