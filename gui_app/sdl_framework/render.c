#include "Render.h"

SDL_AppResult render(App* app) {
    // We only render if we are told so
    if (SDL_GetAtomicInt(&app->state.currentScene.shouldRender) == NO_RERENDER) {
        return SDL_APP_CONTINUE;
    }
    else if (SDL_GetAtomicInt(&app->state.currentScene.shouldRender) == OTHER_THREAD_RERENDER) {
        // If we already have a OTHER_THREAD_RERENDER value then we don't want to rerender twice
        // so we set the value to MAIN_THREAD_RERENDER
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }

    SDL_Color drawColor = app->state.currentScene.sceneRender.renderDrawColor;
    SDL_SetRenderDrawColor(app->state.sdlState.renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
    SDL_RenderClear(app->state.sdlState.renderer);

    SceneRender sceneRender = app->state.currentScene.sceneRender;
    RenderFunction renderFunction;
    SDL_AppResult returnValue;

    // Rendering all the render boxes in the scene
    for (size_t index = 0; index < sceneRender.numRenderBox; index++) {
        renderFunction = sceneRender.renderBoxes[index].renderFunction;
        if (renderFunction) returnValue = renderFunction(sceneRender.renderBoxes[index].renderRect, app);
        if (returnValue != SDL_APP_CONTINUE) return returnValue;
    }

    SDL_RenderPresent(app->state.sdlState.renderer);

    // After we render, we assume that shouldRender is either OTHER_THREAD or MAIN_THREAD
    // This means that if shouldRender is NO_RERENDER then it will stay that way
    if (SDL_GetAtomicInt(&app->state.currentScene.shouldRender) == OTHER_THREAD_RERENDER) {
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }
    else {
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, NO_RERENDER);
    }
    return SDL_APP_CONTINUE;
}
