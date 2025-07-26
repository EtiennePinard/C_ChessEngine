#include "../../../engine/src/utils/FenString.h"

#include "../../sdl_framework/AppCleanup.h"
#include "../../sdl_framework/CommonEvents.h"
#include "../../sdl_framework/CommonRenderFunctions.h"

#include "../Config.h"
#include "../AppStyle.h"
#include "../AppUtils.h"

#include "MainMenuTextInput.h"
#include "MainMenuEvents.h"
#include "MainMenuRender.h"

SDL_AppResult renderPlayerInfoIcon(SDL_FRect rect, App* app, EngineConfig engineConfig, SDL_Color textureColor, int hoverIndex) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    SDL_Color hightLightColor = BUTTON_HIGHLIGHT_COLOR;
    // If this is the currently hovered box, highlight it
    if (app->events.mouseState.hoveredIndex == hoverIndex) {
        SDL_SetRenderDrawColor(renderer, hightLightColor.r, hightLightColor.g, hightLightColor.b, hightLightColor.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_Texture* texture;
    if (engineConfig.isEngine) texture = data->textures.data[1].texture;
    else texture = data->textures.data[0].texture;
    // Changing the texture color. This works because the original texture is white
    SDL_SetTextureColorMod(texture, textureColor.r, textureColor.g, textureColor.b);
    SDL_RenderTexture(renderer, texture, NULL, &rect);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderWhitePlayerInfoIcon(SDL_FRect rect, App* app) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    return renderPlayerInfoIcon(rect, app, data->gameInfo.white.engineConfig, WHITE_COLOR, WHITE_PLAYER_INFO);
}

SDL_AppResult renderBlackPlayerInfoIcon(SDL_FRect rect, App* app) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    return renderPlayerInfoIcon(rect, app, data->gameInfo.black.engineConfig, BLACK_COLOR, BLACK_PLAYER_INFO);
}

SDL_AppResult renderTimeControl(SDL_FRect rect, App* app, TimeControl timeControl, int hoverIndex) {
    char buffer[11];
    formatTimeControl(timeControl, buffer, 11);
    return renderButton(rect, app, hoverIndex, buffer, BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
}

SDL_AppResult renderWhiteTimeControl(SDL_FRect rect, App* app) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    return renderTimeControl(rect, app, data->gameInfo.white.timeControl, WHITE_TIME_CONTROL);
}

SDL_AppResult renderBlackTimeControl(SDL_FRect rect, App* app) {
    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;
    return renderTimeControl(rect, app, data->gameInfo.black.timeControl, BLACK_TIME_CONTROL);
}

SDL_AppResult renderStartingPositionControl(SDL_FRect rect, App* app) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;

    MainMenuSceneData* data = (MainMenuSceneData*)app->state.currentScene.data;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    if (!app->events.textInput.isActive || app->events.textInput.textInputId != STARTING_POSITION_TEXT_INPUT_ID) {
        // Highlight the rectangle if hovered without inputing text
        if (app->events.mouseState.hoveredIndex == STARTING_POSITION) {
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

SDL_AppResult renderStartGameButton(SDL_FRect rect, App* app) {
    return renderButton(rect, app, START_GAME, "Start Game", BUTTON_HIGHLIGHT_COLOR, BUTTON_TEXT_COLOR);
}

#define MM_IMAGE_SIZE_PERCENT (0.12f)
#define MM_BUTTON_HEIGHT_PERCENT (0.07f)
#define MM_BUTTON_WIDTH_PERCENT (0.3f)

SDL_AppResult computeMainMenuSceneRender(App* app) {
    SceneRender* sceneRender = &app->state.currentScene.sceneRender;

    // Setting in the background color of the scene
    sceneRender->renderDrawColor = BACKGROUND_COLOR;

    sceneRender->numRenderBox = TOTAL_MAIN_MENU_RENDER_BOX;
    // We assume that sceneRender->renderBoxes is always NULL
    if (sceneRender->renderBoxes != NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "renderBoxes is not NULL when re-computing game scene\n");
        return SDL_APP_FAILURE;
    }
    sceneRender->renderBoxes = SDL_calloc(sceneRender->numRenderBox, sizeof(RenderBox));

    int windowWidth, windowHeight;
    SDL_GetWindowSize(app->state.sdlState.window, &windowWidth, &windowHeight);

    const float padding = PADDING_PERCENT * windowHeight;
    const float kingImageSize = MM_IMAGE_SIZE_PERCENT * SDL_min(windowWidth, windowHeight);
    const float buttonHeight = MM_BUTTON_HEIGHT_PERCENT * windowHeight;
    const float buttonWidth = MM_BUTTON_WIDTH_PERCENT * windowWidth;
    const float timeControlWidth = buttonWidth;
    const float timeControlHeight = buttonHeight;

    const float centerX = (float)windowWidth / 2.0;
    float y = padding;

    const float totalPlayerInfoWidth = kingImageSize + padding + timeControlWidth;

    float playerInfoX = centerX - totalPlayerInfoWidth / 2;
    SDL_FRect playerInfoRect = {
        .x = playerInfoX,
        .y = y,
        .w = kingImageSize,
        .h = kingImageSize
    };
    playerInfoX += playerInfoRect.w + padding;
    SDL_FRect timeControlRect = {
            .x = playerInfoX,
            .y = y + (kingImageSize - timeControlHeight) / 2,
            .w = timeControlWidth,
            .h = timeControlHeight
    };

    // White player
    sceneRender->renderBoxes[WHITE_PLAYER_INFO].renderRect = playerInfoRect;
    sceneRender->renderBoxes[WHITE_PLAYER_INFO].renderFunction = &renderWhitePlayerInfoIcon;
    sceneRender->renderBoxes[WHITE_PLAYER_INFO].onMouseButtonDown = &clickedWhiteEngineConfig;
    sceneRender->renderBoxes[WHITE_PLAYER_INFO].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[WHITE_PLAYER_INFO].onMouseExited = &rerenderScene;

    sceneRender->renderBoxes[WHITE_TIME_CONTROL].renderRect = timeControlRect;
    sceneRender->renderBoxes[WHITE_TIME_CONTROL].renderFunction = &renderWhiteTimeControl;
    sceneRender->renderBoxes[WHITE_TIME_CONTROL].onMouseButtonDown = &clickedWhiteTimeControl;
    sceneRender->renderBoxes[WHITE_TIME_CONTROL].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[WHITE_TIME_CONTROL].onMouseExited = &rerenderScene;

    y += kingImageSize + padding;

    // Black player
    playerInfoRect.y = y;
    sceneRender->renderBoxes[BLACK_PLAYER_INFO].renderRect = playerInfoRect;
    sceneRender->renderBoxes[BLACK_PLAYER_INFO].renderFunction = &renderBlackPlayerInfoIcon;
    sceneRender->renderBoxes[BLACK_PLAYER_INFO].onMouseButtonDown = &clickedBlackEngineConfig;
    sceneRender->renderBoxes[BLACK_PLAYER_INFO].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BLACK_PLAYER_INFO].onMouseExited = &rerenderScene;

    timeControlRect.y = y + (kingImageSize - timeControlHeight) / 2;
    sceneRender->renderBoxes[BLACK_TIME_CONTROL].renderRect = timeControlRect;
    sceneRender->renderBoxes[BLACK_TIME_CONTROL].renderFunction = &renderBlackTimeControl;
    sceneRender->renderBoxes[BLACK_TIME_CONTROL].onMouseButtonDown = &clickedBlackTimeControl;
    sceneRender->renderBoxes[BLACK_TIME_CONTROL].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BLACK_TIME_CONTROL].onMouseExited = &rerenderScene;

    y += kingImageSize + padding;

    // Other buttons
    const float startingPositionWidth = windowWidth - 2 * padding;
    SDL_FRect startingPosition = {
        .x = centerX - startingPositionWidth / 2,
        .y = y,
        .w = startingPositionWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[STARTING_POSITION].renderRect = startingPosition;
    sceneRender->renderBoxes[STARTING_POSITION].renderFunction = &renderStartingPositionControl;
    sceneRender->renderBoxes[STARTING_POSITION].onMouseEntered = &enteredStartingPosition;
    sceneRender->renderBoxes[STARTING_POSITION].onMouseExited = &exitedStartingPosition;
    sceneRender->renderBoxes[STARTING_POSITION].onMouseButtonDown = &clickedStartingPosition;

    if (app->events.textInput.isActive && app->events.textInput.textInputId == STARTING_POSITION_TEXT_INPUT_ID) {
        app->events.textInput.textInputRender.renderRect = startingPosition;
    }

    y += buttonHeight + padding;

    SDL_FRect startGameButton = {
        .x = centerX - buttonWidth / 2,
        .y = y,
        .w = buttonWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[START_GAME].renderRect = startGameButton;
    sceneRender->renderBoxes[START_GAME].renderFunction = &renderStartGameButton;
    sceneRender->renderBoxes[START_GAME].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[START_GAME].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[START_GAME].onMouseButtonDown = &clickedStartGame;

    // Credits
    const float creditsHeight = CREDITS_HEIGHT_PERCENT * windowHeight;
    const float creditsWidth = CREDITS_WIDTH_PERCENT * windowWidth;
    SDL_FRect creditsRect = {
        .x = centerX - creditsWidth / 2,
        .y = windowHeight - creditsHeight - padding,
        .w = creditsWidth,
        .h = creditsHeight
    };
    sceneRender->renderBoxes[MAIN_MENU_CREDITS].renderRect = creditsRect;
    sceneRender->renderBoxes[MAIN_MENU_CREDITS].renderFunction = &renderCredits;

    return SDL_APP_CONTINUE;
}

void terminateMainMenuScene(void* data) {
    MainMenuSceneData* mainMenuData = (MainMenuSceneData*)data;
    if (!saveMainMenuConfig(&mainMenuData->gameInfo)) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error saving the config file\n");
    if (mainMenuData->gameInfo.white.engineConfig.enginePath != NULL) SDL_free(mainMenuData->gameInfo.white.engineConfig.enginePath);
    if (mainMenuData->gameInfo.black.engineConfig.enginePath != NULL) SDL_free(mainMenuData->gameInfo.black.engineConfig.enginePath);
    cleanupTextures(mainMenuData->textures);
    SDL_free(mainMenuData->textures.data);
    SDL_free(mainMenuData);
}
