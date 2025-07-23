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

    // Handling flashing cursor logic
    if (app->events.textInput.isActive) {
        Uint64 now = SDL_GetTicks();
        if (now - app->events.textInput.cursor.lastCursorToggleTime >= 500) {
            app->events.textInput.cursor.showCursor = !app->events.textInput.cursor.showCursor;
            app->events.textInput.cursor.lastCursorToggleTime = now;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
    }

    return returnValue;
}

SDL_AppResult SDL_AppEvent(void* globalAppObject, SDL_Event* event) {
    return handleEvent((App*)globalAppObject, event);
}

void SDL_AppQuit(void* globalAppObject, SDL_AppResult result) {
    App* app = (App*)globalAppObject;
    if (app->events.modal.isActive && app->events.modal.onEscape) {
        SDL_Event event;
        event.type = SDL_EVENT_QUIT;
        event.quit.timestamp = SDL_GetTicksNS();
        app->events.modal.onEscape(&event, app);
    }
    app->state.currentScene.terminateSceneFunction(app->state.currentScene.data);
    cleanupApp(app);
    free(app);

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
}
