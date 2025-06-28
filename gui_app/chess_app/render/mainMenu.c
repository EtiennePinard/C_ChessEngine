#include <stdlib.h>

#include "../../sdl_framework/AppCleanup.h"

#include "../events/MainMenuEvents.h"
#include "../events/CommonEvents.h"

#include "../Config.h"
#include "../AppStyle.h"

#include "RenderUtils.h"
#include "MainMenu.h"

SDL_AppResult renderWhiteKingImage(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    SDL_RenderTexture(app->state.sdlState.renderer, scene->textures.data[0].texture, NULL, &rect);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderBlackKingImage(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    SDL_RenderTexture(app->state.sdlState.renderer, scene->textures.data[1].texture, NULL, &rect);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderWhitePlayerTypeButton(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    const char* label = scene->gameInfo.white.isEngine ? "Engine" : "Human";
    return renderButton(rect, app, WHITE_PLAYER_TYPE, label);
}

SDL_AppResult renderBlackPlayerTypeButton(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    const char* label = scene->gameInfo.black.isEngine ? "Engine" : "Human";
    return renderButton(rect, app, BLACK_PLAYER_TYPE, label);
}

const char* getFilenameFromPath(const char* path) {
    if (!path) return NULL;

    const char* lastSlash = strrchr(path, '/');
#ifdef _WIN32
    const char* lastBackslash = strrchr(path, '\\');
    if (!lastSlash || (lastBackslash && lastBackslash > lastSlash)) {
        lastSlash = lastBackslash;
    }
#endif

    return lastSlash ? lastSlash + 1 : path;
}

SDL_AppResult renderWhiteEnginePathButton(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    char* label = scene->gameInfo.white.isEngine ? scene->gameInfo.white.enginePath : "Human Player";
    if (!label) label = "Engine's path";
    return renderButton(rect, app, WHITE_ENGINE_PATH, getFilenameFromPath(label));
}

SDL_AppResult renderBlackEnginePathButton(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    char* label = scene->gameInfo.black.isEngine ? scene->gameInfo.black.enginePath : "Human Player";
    if (!label) label = "Engine's path";
    return renderButton(rect, app, BLACK_ENGINE_PATH, getFilenameFromPath(label));
}

SDL_AppResult renderTimeControlLabel(SDL_FRect rect, App* app) {
    TTF_Font* tempFont = TTF_CopyFont(app->state.sdlState.font);
    if (!tempFont) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_CopyFont failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    TTF_SetFontStyle(tempFont, TTF_STYLE_BOLD);
    SDL_AppResult result = renderTextCenteredToFit(app->state.sdlState.renderer, tempFont, "Time Control", false, BUTTON_TEXT_COLOR, rect);
    TTF_CloseFont(tempFont);
    return result;
}

SDL_AppResult renderTimeControlButton(SDL_FRect rect, App* app) {
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    char buffer[11];
    formatTimeControl(scene->gameInfo.timeControl, buffer, 11);
    return renderButton(rect, app, TIME_CONTROL_BUTTON, buffer);
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
    MainMenuSceneData* scene = (MainMenuSceneData*)app->state.currentScene.data;
    if (!scene->timeControlSettings.selectModalVisible) return SDL_APP_CONTINUE;
    scene->timeControlSettings.hovered = (TimeControl){ 0, 0 };

    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_BORDER_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;
    SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color backgroundColor = SEMI_TRANSPARENT_BACKGROUND_COLOR;

    // semi-transparent dark background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &RECT_TO_FRECT(rect));

    // --- Border ---
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &RECT_TO_FRECT(rect));

    const float verticalPadding = rect.h / (2 * NUM_TIME_CONTROL_STYLE);
    const float horizontalPadding = rect.h / (2 * NUM_TIME_CONTROL_OPTIONS_PER_STYLE);
    SDL_FRect optionRect = {
        .x = rect.x + horizontalPadding,
        .y = rect.y + verticalPadding,
        .w = (rect.w - horizontalPadding) / NUM_TIME_CONTROL_OPTIONS_PER_STYLE - horizontalPadding,
        .h = (rect.h - verticalPadding) / NUM_TIME_CONTROL_STYLE - verticalPadding
    };

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_FPoint mousePoint = { mouseX, mouseY };
    for (int styleIndex = 0; styleIndex < NUM_TIME_CONTROL_STYLE; styleIndex++) {
        for (int optionIndex = 0; optionIndex < NUM_TIME_CONTROL_OPTIONS_PER_STYLE; optionIndex++) {
            TimeControl timeControl = timeControlOptions[styleIndex][optionIndex];
            if (app->events.mouseState.hoveredIndex == TIME_CONTROL_MODAL &&
                SDL_PointInRectFloat(&mousePoint, &optionRect)) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_RenderFillRect(renderer, &optionRect);
                scene->timeControlSettings.hovered = timeControl;
            }
            char buffer[11];
            formatTimeControl(timeControl, buffer, 11);
            SDL_AppResult result = renderTextCenteredToFit(
                renderer,
                font,
                buffer,
                false,
                textColor,
                optionRect
            );
            if (result != SDL_APP_CONTINUE) return result;
            optionRect.x += optionRect.w + horizontalPadding;
        }
        optionRect.y += optionRect.h + verticalPadding;
        optionRect.x = rect.x + horizontalPadding;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult renderStartGameButton(SDL_FRect rect, App* app) {
    return renderButton(rect, app, START_GAME, "Start Game");
}

#define COLUMN_WIDTH_PERCENT (0.42f)
#define KING_IMAGE_HEIGHT_PERCENT (0.12f)
#define MM_BUTTON_HEIGHT_PERCENT (0.07f)
#define MM_BUTTON_WIDTH_PERCENT (0.75f)
#define CONTROL_BUTTON_HEIGHT_PERCENT (0.06f)

void computeMainMenuSceneRender(SDL_Window* window, SceneRender* sceneRender) {
    // Setting in the background color of the scene
    sceneRender->renderDrawColor = BACKGROUND_COLOR;

    sceneRender->numRenderBox = TOTAL_MAIN_MENU_RENDER_BOX;
    // We assume that sceneRender->renderBoxes is always NULL
    sceneRender->renderBoxes = calloc(sceneRender->numRenderBox, sizeof(RenderBox));

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    const float padding = PADDING_PERCENT * windowHeight;
    const float columnWidth = COLUMN_WIDTH_PERCENT * windowWidth;
    const float columnSpacing = windowWidth - 2 * (columnWidth + padding);

    const float kingImageSize = KING_IMAGE_HEIGHT_PERCENT * windowHeight;
    const float buttonHeight = MM_BUTTON_HEIGHT_PERCENT * windowHeight;
    const float buttonWidth = MM_BUTTON_WIDTH_PERCENT * columnWidth;
    const float controlButtonHeight = CONTROL_BUTTON_HEIGHT_PERCENT * windowHeight;

    const float centerX = windowWidth / 2;

    float whiteColumnX = padding;
    float blackColumnX = whiteColumnX + columnWidth + columnSpacing;

    // Y offset tracks vertical positioning
    float whiteY = padding;
    float blackY = padding;

    // --- King Images ---
    SDL_FRect whiteKingRect = {
        .x = whiteColumnX + (columnWidth - kingImageSize) / 2,
        .y = whiteY,
        .w = kingImageSize,
        .h = kingImageSize
    };
    sceneRender->renderBoxes[WHITE_KING_IMAGE].renderRect = whiteKingRect;
    sceneRender->renderBoxes[WHITE_KING_IMAGE].renderFunction = &renderWhiteKingImage;
    whiteY += kingImageSize + padding;

    SDL_FRect blackKingRect = {
        .x = blackColumnX + (columnWidth - kingImageSize) / 2,
        .y = blackY,
        .w = kingImageSize,
        .h = kingImageSize
    };
    sceneRender->renderBoxes[BLACK_KING_IMAGE].renderRect = blackKingRect;
    sceneRender->renderBoxes[BLACK_KING_IMAGE].renderFunction = &renderBlackKingImage;
    blackY += kingImageSize + padding;

    // --- Player Type Buttons ---
    SDL_FRect whiteTypeButton = {
        .x = whiteColumnX + (columnWidth - buttonWidth) / 2,
        .y = whiteY,
        .w = buttonWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[WHITE_PLAYER_TYPE].renderRect = whiteTypeButton;
    sceneRender->renderBoxes[WHITE_PLAYER_TYPE].renderFunction = &renderWhitePlayerTypeButton;
    sceneRender->renderBoxes[WHITE_PLAYER_TYPE].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[WHITE_PLAYER_TYPE].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[WHITE_PLAYER_TYPE].onMouseButtonDown = &clickedDownWhitePlayerType;
    whiteY += buttonHeight + padding;

    SDL_FRect blackTypeButton = {
        .x = blackColumnX + (columnWidth - buttonWidth) / 2,
        .y = blackY,
        .w = buttonWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[BLACK_PLAYER_TYPE].renderRect = blackTypeButton;
    sceneRender->renderBoxes[BLACK_PLAYER_TYPE].renderFunction = &renderBlackPlayerTypeButton;
    sceneRender->renderBoxes[BLACK_PLAYER_TYPE].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BLACK_PLAYER_TYPE].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[BLACK_PLAYER_TYPE].onMouseButtonDown = &clickedDownBlackPlayerType;
    blackY += buttonHeight + padding;

    // --- Engine Path Buttons ---
    SDL_FRect whitePathButton = {
        .x = whiteColumnX + (columnWidth - buttonWidth) / 2,
        .y = whiteY,
        .w = buttonWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[WHITE_ENGINE_PATH].renderRect = whitePathButton;
    sceneRender->renderBoxes[WHITE_ENGINE_PATH].renderFunction = &renderWhiteEnginePathButton;
    sceneRender->renderBoxes[WHITE_ENGINE_PATH].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[WHITE_ENGINE_PATH].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[WHITE_ENGINE_PATH].onMouseButtonDown = &clickedDownWhiteEnginePath;
    whiteY += buttonHeight + padding;

    SDL_FRect blackPathButton = {
        .x = blackColumnX + (columnWidth - buttonWidth) / 2,
        .y = blackY,
        .w = buttonWidth,
        .h = buttonHeight
    };
    sceneRender->renderBoxes[BLACK_ENGINE_PATH].renderRect = blackPathButton;
    sceneRender->renderBoxes[BLACK_ENGINE_PATH].renderFunction = &renderBlackEnginePathButton;
    sceneRender->renderBoxes[BLACK_ENGINE_PATH].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[BLACK_ENGINE_PATH].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[BLACK_ENGINE_PATH].onMouseButtonDown = &clickedDownBlackEnginePath;
    blackY += buttonHeight + padding;

    // --- Center Controls ---
    float controlY = SDL_max(blackY, whiteY) + padding;
    float timeControlWidth = 2 * buttonWidth / 3;

    SDL_FRect timeControlButton = {
        .x = centerX - timeControlWidth / 2,
        .y = controlY,
        .w = timeControlWidth,
        .h = controlButtonHeight
    };
    sceneRender->renderBoxes[TIME_CONTROL_BUTTON].renderRect = timeControlButton;
    sceneRender->renderBoxes[TIME_CONTROL_BUTTON].renderFunction = &renderTimeControlButton;
    sceneRender->renderBoxes[TIME_CONTROL_BUTTON].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[TIME_CONTROL_BUTTON].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[TIME_CONTROL_BUTTON].onMouseButtonDown = &clickedDownTimeControlButton;

    float modalWidth = windowWidth - timeControlButton.x - padding;
    float modalHeight = controlButtonHeight * 6; // enough for options
    SDL_FRect timeControlModal = {
        .x = timeControlButton.x,
        .y = controlY,
        .w = modalWidth,
        .h = modalHeight
    };
    sceneRender->renderBoxes[TIME_CONTROL_MODAL].renderRect = timeControlModal;
    sceneRender->renderBoxes[TIME_CONTROL_MODAL].renderFunction = &renderTimeControlModal;
    sceneRender->renderBoxes[TIME_CONTROL_MODAL].onMouseHovered = &rerenderScene;
    sceneRender->renderBoxes[TIME_CONTROL_MODAL].onMouseButtonDown = &clickedDownTimeControlModal;

    controlY += timeControlButton.h + padding;

    SDL_FRect startGameButton = {
        .x = centerX - buttonWidth / 2,
        .y = controlY,
        .w = buttonWidth,
        .h = controlButtonHeight
    };
    sceneRender->renderBoxes[START_GAME].renderRect = startGameButton;
    sceneRender->renderBoxes[START_GAME].renderFunction = &renderStartGameButton;
    sceneRender->renderBoxes[START_GAME].onMouseEntered = &rerenderScene;
    sceneRender->renderBoxes[START_GAME].onMouseExited = &rerenderScene;
    sceneRender->renderBoxes[START_GAME].onMouseButtonUp = &clickedUpStartGame;

    // --- Credits ---
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
}

void terminateMainMenuScene(void* data) {
    MainMenuSceneData* mainMenuData = (MainMenuSceneData*) data;
    if (!saveMainMenuConfig(&mainMenuData->gameInfo)) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error saving the config file\n");
    cleanupTextures(mainMenuData->textures);
    free(mainMenuData->textures.data);
    free(mainMenuData);
}