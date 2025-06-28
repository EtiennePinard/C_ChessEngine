#include "CommonEvents.h"

SDL_AppResult rerenderScene(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event;
    (void)rect;
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}
