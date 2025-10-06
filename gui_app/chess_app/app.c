#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/AppCleanup.h"

#include "../../engine/src/magicBitBoard/MagicBitBoard.h"
#include "../../engine/src/state/ZobristKey.h"

#include "gameScene/GameRender.h"
#include "gameScene/modals/GameModals.h"
#include "gameScene/GameEvents.h"

#include "Config.h"
#include "AppUtils.h"
#include "AppState.h"

SDL_AppResult onWindowResize(SDL_Event* event, App* app) {
    (void)event;
    if (app->state.currentScene.sceneRender.renderBoxes != NULL) {
        SDL_free(app->state.currentScene.sceneRender.renderBoxes);
        app->state.currentScene.sceneRender.renderBoxes = NULL;
    }

    switch (app->state.currentScene.sceneId) {
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
            } else if (app->events.modal.modalId == SETTINGS_MODAL_ID) {
                app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
            } else if (app->events.modal.modalId == TIME_CONTROL_MODAL_ID2) {
                app->events.modal.modalRender.renderRect = calculateSettingsRect(app);
            }
        }
        break;
    default:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Invalid scene ID %d\n", app->state.currentScene.sceneId);
        return SDL_APP_FAILURE;
    }
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult afterRenderAndEventsFunction(App* app) {
    if (app->state.currentScene.sceneId == GAME_SCENE_ID) {
        GameSceneData* data = (GameSceneData*)app->state.currentScene.data;
        
        if (data->state.result == GAME_IS_NOT_DONE) {
            Player* currentPlayer = data->state.position.colorToGo == WHITE ? &data->state.white : &data->state.black;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            
            // Updating the time controls using SDL_GetTicks
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

// TODO: Make only one scene, the game scene and add the main menu as a settings modal
// This will make the app much more intuitive and make the user understand the app without having to poke around too much
bool initializeApp(App* app) {
    // Initializing the SDL libraries and state needed throughout the entire app
    SDL_Log("Initializing SDL libraries...\n");
    if (!initializeSDlLibraries(SDL_INIT_VIDEO) ||
        !initializeSDLState(&app->state.sdlState,
            WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STARTING_WINDOW_WIDTH, STARTING_WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE,
            NULL,
            FONT_PATH, DEFAULT_FONT_SIZE)) {
        return false;
    }
    SDL_Log("Done!\n");

    SDL_Log("Initializing magic bit boards and Zobrist keys...\n");
    // We simply need the move generation and repetition table to work for this app
    if (!MagicBitBoard_init() || !ZobristKey_init()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize the magic bit boards and/or Zobrist keys\n");
        return false;
    }
    SDL_Log("Done!\n");

    SDL_Log("Initializing Game Scene...\n");
    // Loading game config and app style
    GameConfig gameConfig; 
    AppStyle appStyle;
    loadGameConfig(&gameConfig);
    loadAppStyle(&appStyle);
    // Correctly setting game scene data in app
    if (setGameScene(app, &gameConfig, &appStyle) != SDL_APP_CONTINUE) return false;

    SDL_Log("Done!\n");
    
    app->events.modal.isActive = false;
    app->events.textInput.isActive = false;
    app->events.onWindowResize = &onWindowResize;
    app->runAfterRenderAndEventsFunction = &afterRenderAndEventsFunction;

    SDL_Log("App is initialized!\n");
    return true;
}

void cleanupApp(App* app) {
    MagicBitBoard_terminate();

    // Freeing the allocated engine config strings
    switch (app->state.currentScene.sceneId) {
    case GAME_SCENE_ID:
        GameSceneData* gameSceneData = (GameSceneData*)app->state.currentScene.data;
        if (gameSceneData->gameInfo.white.engineConfig.enginePath) SDL_free(gameSceneData->gameInfo.white.engineConfig.enginePath);
        if (gameSceneData->gameInfo.black.engineConfig.enginePath) SDL_free(gameSceneData->gameInfo.black.engineConfig.enginePath);
        if (gameSceneData->gameInfo.startingPositionFen) SDL_free(gameSceneData->gameInfo.startingPositionFen);
        break;
    default:
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Invalid scene ID %d\n", app->state.currentScene.sceneId);
    }

    SDL_free(app->state.currentScene.sceneRender.renderBoxes);

    cleanupSDL_State(app->state.sdlState);
    quitSDL();
}
