#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppStyle.h"
#include "../../../AppUtils.h"

#include "SettingsTextInput.h"
#include "SettingsRender.h"

SDL_AppResult renderPlayerInfoIcon_2(SDL_FRect rect, App* app, EngineConfig engineConfig, SDL_Color textureColor, int hoverIndex) {
    SettingsData* data = (SettingsData*)app->events.modal.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color hightLightColor = BUTTON_HIGHLIGHT_COLOR;

    // Rendering the texture in a square to not distort it
    SDL_FRect squareRect = { rect.x, rect.y, SDL_min(rect.w, rect.h), SDL_min(rect.w, rect.h) };
    squareRect.x += (rect.w - squareRect.w) / 2.0;
    squareRect.y += (rect.h - squareRect.h) / 2.0;

    // If this is the currently hovered box, highlight it
    if (app->events.mouseState.hoveredIndex == hoverIndex &&
        SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &squareRect)) {
        SDL_SetRenderDrawColor(renderer, hightLightColor.r, hightLightColor.g, hightLightColor.b, hightLightColor.a);
        SDL_RenderFillRect(renderer, &squareRect);
    }

    SDL_Texture* texture;
    if (engineConfig.isEngine) texture = data->textures.data[1].texture;
    else texture = data->textures.data[0].texture;
    // Changing the texture color. This works because the original texture is fully white
    SDL_SetTextureColorMod(texture, textureColor.r, textureColor.g, textureColor.b);
    
    SDL_RenderTexture(renderer, texture, NULL, &squareRect);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderTimeControl_2(SDL_FRect rect, App* app, TimeControl timeControl, int hoverIndex) {
    char buffer[11];
    formatTimeControl(timeControl, buffer, 11);
    return renderButton(rect, app, hoverIndex, buffer, BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
}

SDL_AppResult renderStartingPositionControl_2(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    SettingsData* data = (SettingsData*)app->events.modal.data;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    if (!app->events.textInput.isActive || app->events.textInput.textInputId != STARTING_POSITION_TEXT_INPUT_ID) {
        // Highlight the rectangle if hovered without inputing text
        if (app->events.mouseState.hoveredIndex == HOVERING_MODAL &&
            SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->startingPosition)) {
            SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
            SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
            SDL_RenderFillRect(renderer, &rect);
        }
        // Render the text from the game info
        return renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font,
            data->gameInfo.startingPositionFen, textColor, rect);
    }

    return SDL_APP_CONTINUE;
}

#define ENGINE_CONFIG_WIDTH_PERCENT (0.25f)
#define TIME_CONTROL_WIDTH_PERCENT (0.25f)
#define COLOR_TOGGLE_WIDTH_PERCENT (0.25f)
#define ROW_HEIGHT_PERCENT (0.1f)
#define ROW_PADDING_PERCENT (0.1f)
#define BUTTON_WIDTH_PERCENT (0.25f)

SDL_AppResult renderSettingsModal(SDL_FRect rect, App* app) {
    SettingsData* data = (SettingsData*)app->events.modal.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;
    SDL_Color borderColor = BUTTON_BORDER_COLOR;

    // Modal background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &rect);

    // Modal border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    const float padding = PADDING_PERCENT * rect.h;
    float y = rect.y + padding;
    float x = rect.x + padding;
    const float rowHeight = ROW_HEIGHT_PERCENT * rect.h;
    const float rowPadding = ROW_PADDING_PERCENT * rect.h;

    SDL_FRect titleRect = { x, y, rect.w - 2 * padding, rowHeight };
    renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, "Settings", WHITE_COLOR, titleRect);

    y += rowHeight + rowPadding;
    const float engineConfigWidth = ENGINE_CONFIG_WIDTH_PERCENT * rect.w;
    const float timeControlWidth = TIME_CONTROL_WIDTH_PERCENT * rect.w;
    const float colorToggleWidth = COLOR_TOGGLE_WIDTH_PERCENT * rect.w;
    float columnPadding = (rect.w - engineConfigWidth - timeControlWidth - colorToggleWidth) / 4.0;

    x = rect.x + columnPadding;

    SDL_FRect engineConfigRect = { x, y, engineConfigWidth, rowHeight };
    x += engineConfigRect.w + columnPadding;
    SDL_FRect timeControlRect = { x, y, timeControlWidth, rowHeight };
    x += timeControlRect.w + columnPadding;
    SDL_FRect colorToggleRect = { x, y, colorToggleWidth, rowHeight };
    if (data->currentColor == WHITE) {
        renderPlayerInfoIcon_2(engineConfigRect, app, data->gameInfo.white.engineConfig, WHITE_COLOR, HOVERING_MODAL);
        renderTimeControl_2(timeControlRect, app, data->gameInfo.white.timeControl, HOVERING_MODAL);
        renderButton(colorToggleRect, app, HOVERING_MODAL, "White", BUTTON_HIGHLIGHT_COLOR, WHITE_COLOR);
    }
    else {
        renderPlayerInfoIcon_2(engineConfigRect, app, data->gameInfo.black.engineConfig, BLACK_COLOR, HOVERING_MODAL);
        renderTimeControl_2(timeControlRect, app, data->gameInfo.black.timeControl, HOVERING_MODAL);
        renderButton(colorToggleRect, app, HOVERING_MODAL, "Black", BUTTON_HIGHLIGHT_COLOR, BLACK_COLOR);
    }
    data->engineConfig = engineConfigRect;
    data->timeControl = timeControlRect;
    data->colorToggle = colorToggleRect;

    y += rowHeight + rowPadding;
    x = rect.x + padding;
    SDL_FRect startingPositionRect = { x, y, rect.w - 2 * padding, rowHeight };
    renderStartingPositionControl_2(startingPositionRect, app);
    data->startingPosition = startingPositionRect;

    y += rowHeight + rowPadding;
    const float buttonWidth = BUTTON_WIDTH_PERCENT * rect.w;
    columnPadding = (rect.w - 2 * buttonWidth) / 3.0;
    x = rect.x + columnPadding;

    SDL_FRect saveButtonRect = { x, y, buttonWidth, rowHeight };
    x += saveButtonRect.w + columnPadding;
    SDL_FRect cancelButtonRect = { x, y, buttonWidth, rowHeight };
    renderButton(saveButtonRect, app, HOVERING_MODAL, "Save", BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
    renderButton(cancelButtonRect, app, HOVERING_MODAL, "Cancel", BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
    data->saveButton = saveButtonRect;
    data->cancelButton = cancelButtonRect;

    const float creditsHeight = CREDITS_HEIGHT_PERCENT * rect.h;
    const float creditsWidth = CREDITS_WIDTH_PERCENT * rect.w;
    y = rect.y + rect.h - creditsHeight - padding;
    x = rect.x + (rect.w - creditsWidth) / 2.0;
    SDL_FRect creditsRect = { x, y, creditsWidth, creditsHeight };
    renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, CREDIT_COLOR, creditsRect);

    return SDL_APP_CONTINUE;
}
