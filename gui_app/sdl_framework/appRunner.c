#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "AppInit.h"
#include "CommonEvents.h"
#include "AppCleanup.h"

#include "eventHandler.c"
#include "render.c"

SDL_AppResult SDL_AppInit(void** globalAppObject, int argc, char** argv) {
    (void)argc;
    (void)argv;

    App* app = SDL_calloc(1, sizeof(App));

    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, true);
    app->events.shouldHandleEvents = true;
    if (!initializeApp(app)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error when initializing the app\n");
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
    if (app->events.textInput.isActive) {
        SDL_Event event;
        event.type = SDL_EVENT_QUIT;
        event.quit.timestamp = SDL_GetTicksNS();
        if (app->events.textInput.onEscape) app->events.textInput.onEscape(&event, app);
        else closeTextInput(&event, app);
    }
    if (app->events.modal.isActive) {
        SDL_Event event;
        event.type = SDL_EVENT_QUIT;
        event.quit.timestamp = SDL_GetTicksNS();
        if (app->events.modal.onEscape) app->events.modal.onEscape(&event, app);
        else closeModalEventCallback(&event, app);
    }
    app->state.currentScene.terminateSceneFunction(app->state.currentScene.data);
    cleanupApp(app);
    SDL_free(app);

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
