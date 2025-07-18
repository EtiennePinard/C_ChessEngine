#include <stdlib.h>

#include "EventHandler.h"

void appendTextToTextInput(App* app, const char* text) {
    TextInput* ti = &app->events.textInput;
    size_t insertCount = 0;

    for (const char* t = text; *t; t++) {
        // Append only ASCII characters or keep everything if we do not keep only ascii
        // To see why this filtering works, see https://en.wikipedia.org/wiki/UTF-8#Description
        if ((unsigned char)*t < 0x80 || !ti->keepOnlyAscii) insertCount++;
    }

    // Make room in the buffer
    size_t needed = ti->text.count + insertCount + 1;
    if (needed > ti->text.capacity) {
        size_t newCap = needed * 2;
        char* newData = realloc(ti->text.data, newCap);
        SDL_assert(newData);
        ti->text.data = newData;
        ti->text.capacity = newCap;
    }

    // Shift existing data after cursorIndex to the right
    SDL_memmove(
        ti->text.data + ti->cursorIndex + insertCount,
        ti->text.data + ti->cursorIndex,
        ti->text.count - ti->cursorIndex + 1
    );

    // Insert characters
    for (const char* t = text; *t; t++) {
        // Append only ASCII characters or keep everything if we do not keep only ascii
        if ((unsigned char)*text < 0x80 || !ti->keepOnlyAscii) {
            ti->text.data[ti->cursorIndex++] = *t;
            ti->text.count++;
        }
    }
}

SDL_AppResult handleEvent(App* app, SDL_Event* event) {
    if (!app->events.shouldHandleEvents) return SDL_APP_CONTINUE;

    // Updating the mouse point
    SDL_GetMouseState(&app->events.mouseState.mousePoint.x, &app->events.mouseState.mousePoint.y);
    MouseState mouseState = app->events.mouseState;

    size_t index = 0;
    SceneRender sceneRender = app->state.currentScene.sceneRender;
    RenderBox box;

    SceneId currentSceneId = app->state.currentScene.sceneId;

    SDL_AppResult appResult = SDL_APP_CONTINUE;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        int foundIndex = -1;
        if (app->events.modal.isActive) {
            box = app->events.modal.modalRender;
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                foundIndex = HOVERING_MODAL;
                if (mouseState.hoveredIndex != foundIndex && box.onMouseEntered) {
                    appResult = box.onMouseEntered(event, box.renderRect, app);
                }
                if (box.onMouseHovered) appResult = box.onMouseHovered(event, box.renderRect, app);
            }
            // If the scene has changed exit this function
            if (currentSceneId != app->state.currentScene.sceneId) return appResult;
            // If the render boxes have changed exit this function
            if (app->state.currentScene.sceneRender.renderBoxes != sceneRender.renderBoxes) return appResult;

            if (app->events.modal.canOnlyInteractWithModal) goto mouseExited;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                foundIndex = (int)index;

                // Dealing with mouse entered first
                if (mouseState.hoveredIndex != foundIndex && box.onMouseEntered) appResult = box.onMouseEntered(event, box.renderRect, app);
                // Return early if we have encountered an error
                if (appResult != SDL_APP_CONTINUE) return appResult;
                // If the scene has changed exit this function
                if (currentSceneId != app->state.currentScene.sceneId) return appResult;
                // If the render boxes have changed exit this function
                if (app->state.currentScene.sceneRender.renderBoxes != sceneRender.renderBoxes) return appResult;

                // Then dealing with mouse hovered
                if (box.onMouseHovered) appResult = box.onMouseHovered(event, box.renderRect, app);
                // Return early if we have encountered an error
                if (appResult != SDL_APP_CONTINUE) return appResult;
                // If the scene has changed exit this function
                if (currentSceneId != app->state.currentScene.sceneId) return appResult;
                // If the render boxes have changed exit this function
                if (app->state.currentScene.sceneRender.renderBoxes != sceneRender.renderBoxes) return appResult;

                // We assume that only one box is hovered at a time
                break;
            }
        }

    mouseExited:
        if (mouseState.hoveredIndex != -1 && mouseState.hoveredIndex != foundIndex) {
            RenderBox oldBox;
            if (mouseState.hoveredIndex != HOVERING_MODAL) oldBox = sceneRender.renderBoxes[mouseState.hoveredIndex];
            else oldBox = app->events.modal.modalRender;
            if (oldBox.onMouseExited) appResult = oldBox.onMouseExited(event, oldBox.renderRect, app);
        }

        app->events.mouseState.hoveredIndex = foundIndex;

        if (app->events.textInput.isActive) {
            if (app->events.mouseState.holdingLeftMouseButton) {
                // Note: selectionStart is correctly set already
                float mouseX = (float)event->motion.x;

                size_t hoveredIndex = app->events.textInput.text.count;
                for (size_t i = 0; i < app->events.textInput.glyphRects.count; ++i) {
                    SDL_FRect glyphRect = app->events.textInput.glyphRects.data[i];
                    float midX = glyphRect.x + glyphRect.w / 2.0f;
                    if (mouseX < midX) {
                        hoveredIndex = i;
                        break;
                    }
                }

                // Compute selection range
                app->events.textInput.nbCharSelected = (int)hoveredIndex - (int)app->events.textInput.selectionStart;
                app->events.textInput.cursorIndex = app->events.textInput.selectionStart + app->events.textInput.nbCharSelected;
                SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            }
        }

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

        if (app->events.modal.isActive) {
            box = app->events.modal.modalRender;
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (box.onMouseButtonDown) appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else {
                    if (box.onMouseButtonUp) appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
            }
            // If we can only interact with the modal break
            if (app->events.modal.canOnlyInteractWithModal) break;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                // We change the selectedRenderBox on mouse button up and on mouse button down
                app->state.currentScene.selectedRenderBoxIndex = index;
                if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (box.onMouseButtonDown) appResult = box.onMouseButtonDown(event, box.renderRect, app);
                }
                else {
                    if (box.onMouseButtonUp) appResult = box.onMouseButtonUp(event, box.renderRect, app);
                }
                // If a function set a modal to be active and we can only interact with the modal then exit
                if (app->events.modal.isActive && app->events.modal.canOnlyInteractWithModal) break;
                // If the scene has changed exit this function
                if (currentSceneId != app->state.currentScene.sceneId) break;
                // If the render boxes have changed exit this function
                if (app->state.currentScene.sceneRender.renderBoxes != sceneRender.renderBoxes) break;
            }
            // Return early if we have encountered an error
            if (appResult != SDL_APP_CONTINUE) return appResult;
        }

        if (app->events.textInput.isActive) {
            if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event->button.button == SDL_BUTTON_LEFT) {
                    // Reset the nbCharSelected and the selectionStart
                    app->events.textInput.nbCharSelected = 0;
                    app->events.textInput.selectionStart = app->events.textInput.cursorIndex;
                    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
                }
            }
        }

        break;

    case SDL_EVENT_MOUSE_WHEEL:
        if (app->events.modal.isActive) {
            box = app->events.modal.modalRender;
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                if (box.onMouseWheelScrolled) appResult = box.onMouseWheelScrolled(event, box.renderRect, app);
            }
            // If we can only interact with the modal break
            if (app->events.modal.canOnlyInteractWithModal) break;
        }

        for (index = 0; index < sceneRender.numRenderBox; index++) {
            box = sceneRender.renderBoxes[index];
            if (SDL_PointInRectFloat(&mouseState.mousePoint, &box.renderRect)) {
                if (box.onMouseWheelScrolled) appResult = box.onMouseWheelScrolled(event, box.renderRect, app);
                // If a function set a modal to be active and we can only interact with the modal then exit
                if (app->events.modal.isActive && app->events.modal.canOnlyInteractWithModal) break;
            }
            // Return early if we have encountered an error
            if (appResult != SDL_APP_CONTINUE) return appResult;
            // If the scene has changed exit this function
            if (currentSceneId != app->state.currentScene.sceneId) break;
            // If the render boxes have changed exit this function
            if (app->state.currentScene.sceneRender.renderBoxes != sceneRender.renderBoxes) return appResult;
        }
        break;

    case SDL_EVENT_WINDOW_RESIZED:
        if (app->events.onWindowResize) appResult = app->events.onWindowResize(event, app);
        break;
    case SDL_EVENT_KEY_DOWN:
        // First running the custom event handling code
        if (app->events.onKeyDown) appResult = app->events.onKeyDown(event, app);
        if (appResult != SDL_APP_CONTINUE) return appResult;

        // Then cancelling/closing any other active events
        switch (event->key.key) {
        case SDLK_ESCAPE:
            // We first start by cancelling the text input
            if (app->events.textInput.isActive && app->events.textInput.onEscape) {
                app->events.textInput.onEscape(event, app);
            }
            // Then we cancel the modal
            if (app->events.modal.isActive && app->events.modal.onEscape) {
                app->events.modal.onEscape(event, app);
            }
            break;
        case SDLK_RETURN:
            // We first start by closing the text input
            if (app->events.textInput.isActive && app->events.textInput.onReturn) {
                app->events.textInput.onReturn(event, app);
            }
            // Then we close the modal
            if (app->events.modal.isActive && app->events.modal.onReturn) {
                app->events.modal.onReturn(event, app);
            }
            break;
        case SDLK_BACKSPACE:
            if (app->events.textInput.isActive) {
                TextInput* ti = &app->events.textInput;

                if (ti->cursorIndex > 0 && ti->text.count > 0) {
                    // Move text after cursor one char to the left
                    memmove(
                        ti->text.data + ti->cursorIndex - 1,
                        ti->text.data + ti->cursorIndex,
                        ti->text.count - ti->cursorIndex + 1 // includes null terminator
                    );

                    ti->cursorIndex--;
                    ti->text.count--;

                    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
                }
            }
            break;
        case SDLK_V:
            if (app->events.textInput.isActive && (event->key.mod & SDL_KMOD_CTRL)) {
                char* clipboard = SDL_GetClipboardText();
                if (clipboard) {
                    appendTextToTextInput(app, clipboard);
                    SDL_free(clipboard);
                    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
                }
            }
            break;
        case SDLK_LEFT:
            if (app->events.textInput.isActive) {
                TextInput* input = &app->events.textInput;

                if (event->key.mod & SDL_KMOD_SHIFT) {
                    if (input->nbCharSelected == 0) input->selectionStart = input->cursorIndex;
                    if ((int)input->selectionStart + input->nbCharSelected > 0) input->nbCharSelected--;
                    input->cursorIndex = SDL_max((int)input->cursorIndex - 1, 0);
                }
                else {
                    // If shift has stopped being held and something is selected unselect
                    // it and set the cursor position to the start of the selection
                    if (input->nbCharSelected != 0) {
                        // We are holding the arrow in the opposite direction of the selection, setting it to the start of the selection
                        if (input->nbCharSelected > 0) input->cursorIndex = input->selectionStart;

                        input->nbCharSelected = 0;

                    }
                    else {
                        input->cursorIndex = SDL_max((int)input->cursorIndex - 1, 0);
                    }
                }
                // Resetting blinking cursor
                input->showCursor = true;
                input->lastCursorToggleTime = SDL_GetTicks();
                SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            }
            break;
        case SDLK_RIGHT:
            if (app->events.textInput.isActive) {
                TextInput* input = &app->events.textInput;

                if (event->key.mod & SDL_KMOD_SHIFT) {
                    if (input->nbCharSelected == 0) input->selectionStart = input->cursorIndex;
                    if (input->selectionStart + input->nbCharSelected < input->text.count) input->nbCharSelected++;
                    input->cursorIndex = SDL_min(input->cursorIndex + 1, input->text.count);
                }
                else {
                    // If shift has stopped being held and something is selected unselect
                    // it and set the cursor position to the start of the selection
                    if (input->nbCharSelected != 0) {
                        // We are holding the arrow in the opposite direction of the selection, setting it to the start of the selection
                        if (input->nbCharSelected < 0) input->cursorIndex = input->selectionStart;

                        input->nbCharSelected = 0;
                    }
                    else {
                        input->cursorIndex = SDL_min(input->cursorIndex + 1, input->text.count);
                    }
                }
                // Resetting blinking cursor
                input->showCursor = true;
                input->lastCursorToggleTime = SDL_GetTicks();
                SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            }
            break;
        }

        break;
    case SDL_EVENT_TEXT_INPUT:
        // First running the custom event handling code
        if (app->events.onTextInput) appResult = app->events.onTextInput(event, app);

        // Then handling events from the textinput
        if (app->events.textInput.isActive) {
            appendTextToTextInput(app, event->text.text);
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }

        break;
    default: break;
    }
    return appResult;
}
