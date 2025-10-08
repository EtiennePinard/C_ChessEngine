#include "../../../../sdl_framework/CommonRenderFunctions.h"

#include "../../../AppStyle.h"
#include "../../../AppUtils.h"

#include "SettingsTextInput.h"
#include "SettingsRender.h"

SDL_AppResult renderPlayerInfoIcon(SDL_FRect rect, App* app, EngineConfig engineConfig, SDL_Color textureColor, int hoverIndex) {
    SettingsData* data = (SettingsData*)app->events.modal.data;
    GameSceneData* sceneData = (GameSceneData*)app->state.currentScene.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color hightLightColor = sceneData->appStyle.buttonStyle.hoverColor;

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

SDL_AppResult renderTimeControl(SDL_FRect rect, App* app, TimeControl timeControl, int hoverIndex) {
    char buffer[11];
    formatTimeControl(timeControl, buffer, 11);
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;
    return renderButton(rect, app, hoverIndex, buffer,
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor);
}

SDL_AppResult renderStartingPositionControl(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    SettingsData* data = (SettingsData*)app->events.modal.data;
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;

    SDL_Color borderColor = style.buttonStyle.borderColor;
    SDL_Color textColor = style.textStyle.textColor;

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    if (!app->events.textInput.isActive || app->events.textInput.textInputId != STARTING_POSITION_TEXT_INPUT_ID) {
        // Highlight the rectangle if hovered without inputing text
        if (app->events.mouseState.hoveredIndex == HOVERING_MODAL &&
            SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &data->startingPosition)) {
            SDL_Color highlightColor = style.buttonStyle.hoverColor;
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
    AppStyle style = ((GameSceneData*)app->state.currentScene.data)->appStyle;

    SDL_Renderer* renderer = app->state.sdlState.renderer;

    SDL_Color backgroundColor = style.backgroundColor;
    SDL_Color borderColor = style.buttonStyle.borderColor;

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
    renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, "Settings", 
        style.textStyle.textColor, titleRect);

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
        renderPlayerInfoIcon(engineConfigRect, app, data->gameInfo.white.engineConfig, style.textStyle.textColor, HOVERING_MODAL);
        renderTimeControl(timeControlRect, app, data->gameInfo.white.timeControl, HOVERING_MODAL);
        renderButton(colorToggleRect, app, HOVERING_MODAL, "White",
            style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
            style.buttonStyle.idleColor, style.buttonStyle.borderColor,
            style.textStyle.textColor);
    }
    else {
        renderPlayerInfoIcon(engineConfigRect, app, data->gameInfo.black.engineConfig, BLACK_COLOR, HOVERING_MODAL);
        renderTimeControl(timeControlRect, app, data->gameInfo.black.timeControl, HOVERING_MODAL);
        renderButton(colorToggleRect, app, HOVERING_MODAL, "Black",
            style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
            style.buttonStyle.idleColor, style.buttonStyle.borderColor, BLACK_COLOR);
    }
    data->engineConfig = engineConfigRect;
    data->timeControl = timeControlRect;
    data->colorToggle = colorToggleRect;

    y += rowHeight + rowPadding;
    x = rect.x + padding;
    SDL_FRect startingPositionRect = { x, y, rect.w - 2 * padding, rowHeight };
    renderStartingPositionControl(startingPositionRect, app);
    data->startingPosition = startingPositionRect;

    y += rowHeight + rowPadding / 2;
    const float buttonWidth = BUTTON_WIDTH_PERCENT * rect.w;
    x = rect.x + (rect.w - buttonWidth) / 2;
    SDL_FRect styleButtonRect = { x, y, buttonWidth, rowHeight };
    renderButton(styleButtonRect, app, HOVERING_MODAL, "Change style",
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor
    );
    data->styleButton = styleButtonRect;
    y += rowHeight + rowPadding / 2;

    columnPadding = (rect.w - 2 * buttonWidth) / 3.0;
    x = rect.x + columnPadding;

    SDL_FRect saveButtonRect = { x, y, buttonWidth, rowHeight };
    x += saveButtonRect.w + columnPadding;
    SDL_FRect cancelButtonRect = { x, y, buttonWidth, rowHeight };
    renderButton(saveButtonRect, app, HOVERING_MODAL, "Save",
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor);
    renderButton(cancelButtonRect, app, HOVERING_MODAL, "Cancel",
        style.buttonStyle.hoverColor, style.buttonStyle.clickedColor,
        style.buttonStyle.idleColor, style.buttonStyle.borderColor,
        style.textStyle.textColor);
    data->saveButton = saveButtonRect;
    data->cancelButton = cancelButtonRect;

    const float creditsHeight = CREDITS_HEIGHT_PERCENT * rect.h;
    const float creditsWidth = CREDITS_WIDTH_PERCENT * rect.w;
    y = rect.y + rect.h - creditsHeight - padding;
    x = rect.x + (rect.w - creditsWidth) / 2.0;
    SDL_FRect creditsRect = { x, y, creditsWidth, creditsHeight };
    renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, style.creditsColor, creditsRect);

    return SDL_APP_CONTINUE;
}
