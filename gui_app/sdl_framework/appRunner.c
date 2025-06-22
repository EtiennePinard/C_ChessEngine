#include <stdlib.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "AppInit.h"
#include "EventHandler.h"
#include "Render.h"
#include "AppCleanup.h"

SDL_AppResult SDL_AppInit(void** globalAppObject, int argc, char** argv) {
    (void)argc;
    (void)argv;

    App* app = calloc(1, sizeof(App));

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, true);
    app->events.shouldHandleEvents = true;
    if (!initializeApp(app)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when initalizing the app\n");
        return SDL_APP_FAILURE;
    }

    *globalAppObject = app;

    // We are officially running the app!
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* globalAppObject) {
    App* app = (App*)globalAppObject;

    SDL_AppResult returnValue;
    returnValue = render(app);
    if (returnValue != SDL_APP_CONTINUE) return returnValue;

    if (app->runAfterRenderAndEventsFunction) returnValue = app->runAfterRenderAndEventsFunction(app);
    return returnValue;
}

SDL_AppResult SDL_AppEvent(void* globalAppObject, SDL_Event* event) {
    return handleEvent((App*)globalAppObject, event);
}

void SDL_AppQuit(void* globalAppObject, SDL_AppResult result) {
    switch (result) {
    case SDL_APP_FAILURE:
        SDL_Log("The app was terminated with an error\n");
        break;
    case SDL_APP_SUCCESS:
        SDL_Log("The app was terminated with success\n");
        break;
    default:
        break;
    }

    App* app = (App*)globalAppObject;
    app->state.currentScene.terminateSceneFunction(app->state.currentScene.data);
    cleanupApp(app);
    free(app);
}
