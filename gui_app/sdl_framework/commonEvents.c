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
    free(app->events.textInput.text.data);
    free(app->events.textInput.glyphRects.data);
    app->events.textInput.isActive = false;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    if (!SDL_StopTextInput(app->state.sdlState.window)) return SDL_APP_FAILURE;
    else return SDL_APP_CONTINUE;
}

SDL_AppResult clickToCursor(App* app) {
    TextInput* input = &app->events.textInput;

    // Find closest character index
    size_t newIndex = 0;
    for (size_t i = 0; i < input->glyphRects.count; ++i) {
        SDL_FRect glyph = input->glyphRects.data[i];
        float centerX = glyph.x + glyph.w / 2.0f;
        if (app->events.mouseState.mousePoint.x < centerX) {
            newIndex = i;
            break;
        }
        newIndex = i + 1; // if click is past all centers, place at end
    }

    input->cursorIndex = newIndex;
    app->events.textInput.showCursor = true; // Resetting show cursor
    app->events.textInput.lastCursorToggleTime = SDL_GetTicks();
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult changeMouseIconOnEnterTextInput(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    if (!app->events.textInput.isActive) return SDL_APP_CONTINUE;

    app->events.textInput.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    SDL_SetCursor(app->events.textInput.cursor);

    return SDL_APP_CONTINUE;
}

SDL_AppResult resetMouseIconOnExitTextInput(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;
    if (!app->events.textInput.isActive) return SDL_APP_CONTINUE;

    SDL_DestroyCursor(app->events.textInput.cursor);
    app->events.textInput.cursor = SDL_GetDefaultCursor();
    SDL_SetCursor(app->events.textInput.cursor);
    
    return SDL_APP_CONTINUE;
}
