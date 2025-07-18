#include <stdlib.h>

#include "TextInput.h"

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

SDL_AppResult textInputMouseMotion(SDL_Event* event, App* app) {
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
    return SDL_APP_CONTINUE;
}

SDL_AppResult textInputMouseButtonPressed(SDL_Event* event, App* app) {
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
    return SDL_APP_CONTINUE;
}

SDL_AppResult textInputKeyDown(SDL_Event* event, App* app) {
    if (!app->events.textInput.isActive) return SDL_APP_CONTINUE;

    TextInput* input = &app->events.textInput;;

    switch (event->key.key) {
    case SDLK_ESCAPE:
        // We first start by cancelling the text input
        if (app->events.textInput.onEscape) app->events.textInput.onEscape(event, app);
        break;
    case SDLK_RETURN:
        // We first start by closing the text input
        if (app->events.textInput.onReturn) app->events.textInput.onReturn(event, app);
        break;
    case SDLK_BACKSPACE:
        if (input->nbCharSelected != 0) {
            size_t selectionStart = SDL_min(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);
            size_t selectionEnd = SDL_max(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);

            // Move text after selectionEnd to selectionEnd
            memmove(
                input->text.data + selectionStart,
                input->text.data + selectionEnd,
                input->text.count - selectionEnd + 1 // includes null terminator
            );

            input->text.count -= (selectionEnd - selectionStart);
            input->cursorIndex = selectionStart;

            input->nbCharSelected = 0;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
        else if (input->cursorIndex > 0 && input->text.count > 0) {
            // Move text after cursor one char to the left
            memmove(
                input->text.data + input->cursorIndex - 1,
                input->text.data + input->cursorIndex,
                input->text.count - input->cursorIndex + 1 // includes null terminator
            );

            input->cursorIndex--;
            input->text.count--;

            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
        break;
    case SDLK_V:
        if (event->key.mod & SDL_KMOD_CTRL) {
            char* clipboard = SDL_GetClipboardText();
            if (clipboard) {
                appendTextToTextInput(app, clipboard);
                SDL_free(clipboard);
                SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            }
        }
        break;
    case SDLK_LEFT:
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

        break;
    case SDLK_RIGHT:
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
        break;
    default: break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult textInputTextInputEvent(SDL_Event* event, App* app) {
    if (app->events.textInput.isActive) {
        appendTextToTextInput(app, event->text.text);
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }
    return SDL_APP_CONTINUE;
}
