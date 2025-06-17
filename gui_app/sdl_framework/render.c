#include "Render.h"

SDL_AppResult render(App* app) {
    // We only render if we are told so
    if (!app->state.currentScene.shouldRender) return SDL_APP_CONTINUE;
    
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

    // After we render, we assume the content will not change so we set shouldRender to false
    app->state.currentScene.shouldRender = false;
    return SDL_APP_CONTINUE;
}
