#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/AppCleanup.h"

#include "../../engine/src/magicBitBoard/MagicBitBoard.h"
#include "../../engine/src/state/ZobristKey.h"
#include "../../engine/src/bot/PieceSquareTable.h"
#include "../../engine/src/bot/TranspositionTable.h"
#include "../../engine/src/utils/FenString.h"

#include "uciEngineCommunication/UCIEngineCommunication.h"
#include "render/GameScene.h"
#include "render/MainMenu.h"
#include "Config.h"
#include "AppState.h"

SDL_AppResult onWindowResize(App* app, SDL_Event* event) {
    (void)event;
    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }

    switch (app->state.currentScene.sceneId) {
    case GAME_SCENE_ID:
        computeGameSceneRender(app->state.sdlState.window, &app->state.currentScene);
        break;
    case MAIN_MENU_SCENE_ID:
        computeMainMenuSceneRender(app->state.sdlState.window, &app->state.currentScene.sceneRender);
        break;
    default: break;
    }
    app->state.currentScene.shouldRender = true;
    return SDL_APP_CONTINUE;
}

SDL_AppResult afterRenderAndEventsFunction(App* app) {
    // Simply handling the time controls for now
    if (app->state.currentScene.sceneId != GAME_SCENE_ID) return SDL_APP_CONTINUE;

    // Updating the time controls using SDL_GetTicks
    GameSceneData* data = (GameSceneData*)app->state.currentScene.data; 
    Player* currentPlayer = data->state.position.colorToGo == WHITE ? &data->state.white : &data->state.black;

    if (data->state.gameEndedSettings.result == GAME_IS_NOT_DONE) {
        app->state.currentScene.shouldRender = true;
        u64 currentTick = SDL_GetTicks();
        if (currentPlayer->timeControl.timeLeft <= currentTick - data->state.previousTick) {
            currentPlayer->timeControl.timeLeft = 0;
            data->state.gameEndedSettings.result = data->state.position.colorToGo == WHITE ? BLACK_WON_ON_TIME : WHITE_WON_ON_TIME;
        } else {
            currentPlayer->timeControl.timeLeft -= (currentTick - data->state.previousTick);
        }
        data->state.previousTick = currentTick;
    }


    return SDL_APP_CONTINUE;
}

bool initializeApp(App* app) {
    // Initializing the SDL libraries and state needed throughout the entire app
    printf("Initializing SDL libraries... ");
    if (!initializeSDlLibraries(SDL_INIT_VIDEO) ||
        !initializeSDLState(&app->state.sdlState,
            TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
            NULL,
            FONT_PATH, DEFAULT_FONT_SIZE)) {
        return false;
    }
    printf("Done!\n");

    printf("Initializing magic bit boards and Zobrist keys... ");
    // We simply need the move generation and repetition table to work for this app
    if (!MagicBitBoard_init() || !ZobristKey_init()) {
        fprintf(stderr, "Failed to initialize the magic bit boards and/or Zobrist keys\n");
        return false;
    }
    printf("Done!\n");

    printf("Initializing Main Menu Scene... ");
    MainMenuSceneData* mainMenu = calloc(1, sizeof(MainMenuSceneData));
    loadMainMenuConfig(&mainMenu->gameSettings);
    mainMenu->timeControlSettings.selectModalVisible = false;
    mainMenu->timeControlSettings.hovered = (TimeControl){ 0, 0 };

    const char* kingImages[2] = { WHITE_KING_IMG_PATH, BLACK_KING_IMG_PATH };
    if (!initializeTextures(&mainMenu->textures, 2) ||
        !loadImageFromFilePath(&app->state.sdlState, &mainMenu->textures, kingImages, 2)) {
        return false;
    }

    app->state.currentScene.data = mainMenu;
    app->state.currentScene.sceneId = MAIN_MENU_SCENE_ID;
    computeMainMenuSceneRender(app->state.sdlState.window, &app->state.currentScene.sceneRender);

    app->events.onWindowResize = &onWindowResize;
    app->runAfterRenderAndEventsFunction = &afterRenderAndEventsFunction;

    printf("Done!\n");

    printf("App is initialized!\n");
    return true;
}

void cleanupApp(App* app) {
    MagicBitBoard_terminate();

    cleanupSDL_State(app->state.sdlState);
    quitSDL();
}
