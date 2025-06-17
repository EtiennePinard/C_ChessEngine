#include "CommonEvents.h"

SDL_AppResult rerenderScene(SDL_Event* event, SDL_Rect rect, App* app) {
    (void)event;
    (void)rect;
    app->state.currentScene.shouldRender = true;
    return SDL_APP_CONTINUE;
}
