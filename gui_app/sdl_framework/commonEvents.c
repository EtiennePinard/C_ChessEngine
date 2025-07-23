#include <stdlib.h>

#include "CommonEvents.h"

SDL_AppResult rerenderScene(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult closeModalEventCallbackBox(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    free(app->events.modal.data);
    app->events.modal.isActive = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult closeModalEventCallback(SDL_Event* event, App* app) {
    (void)event;
    free(app->events.modal.data);
    app->events.modal.isActive = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult closeTextInput(SDL_Event* event, App* app) {
    (void)event;
    if (app->events.textInput.text.data) {
        free(app->events.textInput.text.data);
        app->events.textInput.text.data = NULL;
        app->events.textInput.text.count = 0;
        app->events.textInput.text.capacity = 0;
    }
    if (app->events.textInput.glyphRects.data) {
        free(app->events.textInput.glyphRects.data);
        app->events.textInput.glyphRects.data = NULL;
        app->events.textInput.glyphRects.count = 0;
        app->events.textInput.glyphRects.capacity = 0;
    }
    app->events.textInput.isActive = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    if (!SDL_StopTextInput(app->state.sdlState.window)) return SDL_APP_FAILURE;
    else return SDL_APP_CONTINUE;
}
