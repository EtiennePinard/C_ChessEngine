#include "EventHandler.h"

static bool pointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

SDL_AppResult handleEvent(App* app, SDL_Event* event) {
    if (!app->events.shouldHandleEvents) return SDL_APP_CONTINUE;

    float mouseX, mouseY;
    size_t index = 0;
    SceneRender sceneRender = app->state.currentScene.sceneRender;
    RenderBox box;
    SDL_AppResult appResult = SDL_APP_CONTINUE;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        SDL_GetMouseState(&mouseX, &mouseY);
        MouseState mouseState = app->events.mouseState;
        int foundIndex = -1;
        if (app->events.lockSelectedBoxIndex) {
            box = sceneRender.renderBoxes[app->state.currentScene.selectedRenderBoxIndex];
            if (pointInRect((int)mouseX, (int)mouseY, box.renderRect)) {
                foundIndex = app->state.currentScene.selectedRenderBoxIndex;
                if (mouseState.hoveredIndex != foundIndex && box.onMouseEntered) {
                    appResult = box.onMouseEntered(event, box.renderRect, app);
                }
                if (box.onMouseHovered) appResult = box.onMouseHovered(event, box.renderRect, app);
            }
            goto mouseExited;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (pointInRect((int)mouseX, (int)mouseY, box.renderRect)) {
                foundIndex = (int)index;

                if (mouseState.hoveredIndex != foundIndex && box.onMouseEntered) {
                    appResult = box.onMouseEntered(event, box.renderRect, app);
                }
                if (box.onMouseHovered) appResult = box.onMouseHovered(event, box.renderRect, app);

                break; // Only one box should be hovered at a time
            }
        }

mouseExited:
        if (mouseState.hoveredIndex != -1 && mouseState.hoveredIndex != foundIndex) {
            RenderBox oldBox = sceneRender.renderBoxes[mouseState.hoveredIndex];
            if (oldBox.onMouseExited) appResult = oldBox.onMouseExited(event, oldBox.renderRect, app);
        }

        app->events.mouseState.hoveredIndex = foundIndex;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        SDL_GetMouseState(&mouseX, &mouseY);
        if (app->events.lockSelectedBoxIndex) {
            box = sceneRender.renderBoxes[app->state.currentScene.selectedRenderBoxIndex];
            if (pointInRect((int)mouseX, (int)mouseY, box.renderRect)) {
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && box.onMouseButtonDown) {
                    appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && box.onMouseButtonUp) {
                    appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
            }
            break;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (pointInRect((int)mouseX, (int)mouseY, box.renderRect)) {
                // We change the selectedRenderBox on mouse button up and on mouse button down
                app->state.currentScene.selectedRenderBoxIndex = index;
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && box.onMouseButtonDown) {
                    appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP && box.onMouseButtonUp) {
                    appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
            }
        }
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        if (app->events.onWindowResize) appResult = app->events.onWindowResize(app, event);
        break;
    default: break;
    }
    return appResult;
}
