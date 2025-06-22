#include "EventHandler.h"

SDL_AppResult handleEvent(App* app, SDL_Event* event) {
    if (!app->events.shouldHandleEvents) return SDL_APP_CONTINUE;

    MouseState mouseState;

    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Point mousePoint = { (int)mouseX, (int)mouseY };

    size_t index = 0;
    SceneRender sceneRender = app->state.currentScene.sceneRender;
    RenderBox box;
    SDL_AppResult appResult = SDL_APP_CONTINUE;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        mouseState = app->events.mouseState;
        int foundIndex = -1;
        if (app->events.lockSelectedBoxIndex) {
            box = sceneRender.renderBoxes[app->state.currentScene.selectedRenderBoxIndex];
            if (SDL_PointInRect(&mousePoint, &box.renderRect)) {
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
            if (SDL_PointInRect(&mousePoint, &box.renderRect)) {
                foundIndex = (int)index;

                if (mouseState.hoveredIndex != foundIndex && box.onMouseEntered) appResult = box.onMouseEntered(event, box.renderRect, app);
                // Return early if we have encountered an error
                if (appResult != SDL_APP_CONTINUE) return appResult;
                if (box.onMouseHovered) appResult = box.onMouseHovered(event, box.renderRect, app);
                // Return early if we have encountered an error
                if (appResult != SDL_APP_CONTINUE) return appResult;
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
        if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event->button.button == SDL_BUTTON_LEFT) {
                app->events.mouseState.holdingLeftMouseButton = event->button.down;
            }
            else if (event->button.button == SDL_BUTTON_RIGHT) {
                app->events.mouseState.holdingRightMouseButton = event->button.down;
            }
        }
        else /* MOUSE_BUTTON_UP */ {
            if (event->button.button == SDL_BUTTON_LEFT) {
                app->events.mouseState.holdingLeftMouseButton = false;
            }
            else if (event->button.button == SDL_BUTTON_RIGHT) {
                app->events.mouseState.holdingRightMouseButton = false;
            }
        }

        if (app->events.lockSelectedBoxIndex) {
            box = sceneRender.renderBoxes[app->state.currentScene.selectedRenderBoxIndex];
            if (SDL_PointInRect(&mousePoint, &box.renderRect)) {
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                   if (box.onMouseButtonDown) appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else {
                    if (box.onMouseButtonUp) appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
            }
            break;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (SDL_PointInRect(&mousePoint, &box.renderRect)) {
                // We change the selectedRenderBox on mouse button up and on mouse button down
                app->state.currentScene.selectedRenderBoxIndex = index;
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                   if (box.onMouseButtonDown) appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else {
                    if (box.onMouseButtonUp) appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
            }
            // Return early if we have encountered an error
            if (appResult != SDL_APP_CONTINUE) return appResult;
        }
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        if (app->events.onWindowResize) appResult = app->events.onWindowResize(app, event);
        break;
    default: break;
    }
    return appResult;
}
