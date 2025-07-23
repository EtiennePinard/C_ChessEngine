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
#include "gameScene/GameRender.h"
#include "mainMenuScene/MainMenuRender.h"
#include "gameScene/GameModals.h"
#include "mainMenuScene/MainMenuModals.h"
#include "gameScene/GameEvents.h"
#include "Config.h"
#include "AppState.h"

SDL_AppResult onWindowResize(SDL_Event* event, App* app) {
    (void)event;
    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }

    switch (app->state.currentScene.sceneId) {
    case MAIN_MENU_SCENE_ID:
        if (computeMainMenuSceneRender(app) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
        if (app->events.modal.isActive) {
            app->events.modal.modalRender.renderRect = computeMainMenuModalRect(app->state.sdlState.window);
            GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
            if (app->events.modal.modalId == PROMOTION_MODAL_ID) {
                PromotionModalData* modalData = (PromotionModalData*)app->events.modal.data;
                app->events.modal.modalRender.renderRect = calculatePromotionRect(data, modalData->promotionSquareTo, app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
            }
            else if (app->events.modal.modalId == GAME_ENDED_MODAL_ID) {
                app->events.modal.modalRender.renderRect = calculateGameEndedRect(app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
            }
        }
        break;
    case GAME_SCENE_ID:
        if (computeGameSceneRender(app) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
        if (app->events.modal.isActive) {
            GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
            if (app->events.modal.modalId == PROMOTION_MODAL_ID) {
                PromotionModalData* modalData = (PromotionModalData*)app->events.modal.data;
                app->events.modal.modalRender.renderRect = calculatePromotionRect(data, modalData->promotionSquareTo, app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
            }
            else if (app->events.modal.modalId == GAME_ENDED_MODAL_ID) {
                app->events.modal.modalRender.renderRect = calculateGameEndedRect(app->state.currentScene.sceneRender.renderBoxes[CHESSBOARD].renderRect);
            }
        }
        break;
    default: break;
    }
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult afterRenderAndEventsFunction(App* app) {
    if (app->state.currentScene.sceneId == GAME_SCENE_ID) {
        // Updating the time controls using SDL_GetTicks
        GameSceneData* data = (GameSceneData*)app->state.currentScene.data;

        if (data->state.result == GAME_IS_NOT_DONE) {
            Player* currentPlayer = data->state.position.colorToGo == WHITE ? &data->state.white : &data->state.black;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

            u64 currentTick = SDL_GetTicks();
            if (currentPlayer->timeControl.timeLeft <= currentTick - data->state.previousTick) {
                currentPlayer->timeControl.timeLeft = 0;
                data->state.result = data->state.position.colorToGo == WHITE ? BLACK_WON_ON_TIME : WHITE_WON_ON_TIME;
                setGameEndedModalActive(app);
            }
            else {
                currentPlayer->timeControl.timeLeft -= (currentTick - data->state.previousTick);
            }
            data->state.previousTick = currentTick;

            // Playing the bot moves if the current player is a bot
            // We do this here so that we don't have to deal with it in
            // every game events which plays a move
            if (currentPlayer->engineCommunication != NULL && !SDL_GetAtomicInt(&currentPlayer->isBotThinking)) {
                SDL_Thread* thread = playBotMove(app);
                if (!thread) return SDL_APP_FAILURE;
                SDL_DetachThread(thread);
            }
        }
    }

    return SDL_APP_CONTINUE;
}

bool initializeApp(App* app) {
    // Initializing the SDL libraries and state needed throughout the entire app
    printf("Initializing SDL libraries... ");
    if (!initializeSDlLibraries(SDL_INIT_VIDEO) ||
        !initializeSDLState(&app->state.sdlState,
            WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
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
    loadMainMenuConfig(&mainMenu->gameInfo);
    app->events.modal.isActive = false;

    const char* mainMenuImages[2] = { HUMAN_ICON_PATH, COMPUTER_ICON_PATH };
    if (!initializeTextures(&mainMenu->textures, 2) ||
        !loadImageFromFilePath(&app->state.sdlState, &mainMenu->textures, mainMenuImages, 2)) {
        return false;
    }

    app->state.currentScene.data = mainMenu;
    app->state.currentScene.sceneId = MAIN_MENU_SCENE_ID;
    app->state.currentScene.terminateSceneFunction = &terminateMainMenuScene;
    if (computeMainMenuSceneRender(app) != SDL_APP_CONTINUE) return false;

    app->events.onWindowResize = &onWindowResize;
    app->runAfterRenderAndEventsFunction = &afterRenderAndEventsFunction;

    printf("Done!\n");

    printf("App is initialized!\n");
    return true;
}

void cleanupApp(App* app) {
    MagicBitBoard_terminate();

    free(app->state.currentScene.sceneRender.renderBoxes);

    cleanupSDL_State(app->state.sdlState);
    quitSDL();
}
