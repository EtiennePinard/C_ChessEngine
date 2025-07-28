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
        char* newData = SDL_realloc(ti->text.data, newCap);
        SDL_assert(newData);
        ti->text.data = newData;
        ti->text.capacity = newCap;
    }

    // Shift existing data after the cursor's index to the right
    SDL_memmove(
        ti->text.data + ti->cursor.index + insertCount,
        ti->text.data + ti->cursor.index,
        ti->text.count - ti->cursor.index + 1
    );

    // Insert characters
    for (const char* t = text; *t; t++) {
        // Append only ASCII characters or keep everything if we do not keep only ascii
        if ((unsigned char)*text < 0x80 || !ti->keepOnlyAscii) {
            ti->text.data[ti->cursor.index++] = *t;
            ti->text.count++;
        }
    }
}

/**
 * @brief Removes amountToRemove number of characters from startingIndex to the text input.
 * 
 * @param app The app's data
 * @param startingIndex The index to start from to remove characters
 * @param amountToRemove The amount of characters to remove, can be negative
 */
void removeTextFromSelectionStartToTextInput(App* app, size_t startingIndex, int amountToRemove) {
    TextInput* input = &app->events.textInput;

    size_t start = SDL_min(startingIndex, startingIndex + amountToRemove);
    size_t end = SDL_max(startingIndex, startingIndex + amountToRemove);
    
    SDL_assert(start < input->text.count && end <= input->text.count);

    // Move text after selectionEnd to selectionEnd
    SDL_memmove(
        input->text.data + start,
        input->text.data + end,
        input->text.count - end + 1 // includes null terminator
    );

    input->text.count -= (end - start);
}

void resetCursorBlink(App* app) {
    app->events.textInput.cursor.showCursor = true;
    app->events.textInput.cursor.lastCursorToggleTime = SDL_GetTicks();
}

SDL_AppResult updateTextInputSelectionOnMouseHovered(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)rect;

    if (app->events.mouseState.holdingLeftMouseButton) {
        // Note: we assume that selectionStart is correctly set already
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
        app->events.textInput.cursor.index = app->events.textInput.selectionStart + app->events.textInput.nbCharSelected;
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
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
            removeTextFromSelectionStartToTextInput(app, input->selectionStart, input->nbCharSelected);
            input->cursor.index = SDL_min(input->selectionStart, input->selectionStart + input->nbCharSelected);
            input->nbCharSelected = 0;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
        else if (input->cursor.index > 0 && input->text.count > 0) {
            removeTextFromSelectionStartToTextInput(app, input->cursor.index, -1);
            input->cursor.index--;
            SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        }
        break;
    case SDLK_V:
        if (event->key.mod & SDL_KMOD_CTRL) {
            char* clipboard = SDL_GetClipboardText();
            if (clipboard) {
                if (input->nbCharSelected != 0) {
                    removeTextFromSelectionStartToTextInput(app, input->selectionStart, input->nbCharSelected);
                    input->cursor.index = SDL_min(input->selectionStart, input->selectionStart + input->nbCharSelected);
                    input->nbCharSelected = 0;
                }
                appendTextToTextInput(app, clipboard);
                SDL_free(clipboard);
                SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
            }
        }
        break;
    case SDLK_C:
        if (event->key.mod & SDL_KMOD_CTRL) {
            if (input->nbCharSelected != 0) {
                // Normalize selection range
                size_t selectionStart = SDL_min(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);
                size_t selectionEnd = SDL_max(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);
                size_t size = selectionEnd - selectionStart;
                char copiedText[size + 1];
                SDL_memcpy(copiedText, input->text.data + selectionStart, size);
                copiedText[size] = '\0';
                if (!SDL_SetClipboardText(copiedText)) return SDL_APP_FAILURE;
            }
        }
        break;
    case SDLK_LEFT:
        if (event->key.mod & SDL_KMOD_SHIFT) {
            if (input->nbCharSelected == 0) input->selectionStart = input->cursor.index;
            if ((int)input->selectionStart + input->nbCharSelected > 0) input->nbCharSelected--;
            input->cursor.index = SDL_max((int)input->cursor.index - 1, 0);
        }
        else {
            // If shift has stopped being held and something is selected unselect
            // it and set the cursor position to the start of the selection
            if (input->nbCharSelected != 0) {
                // We are holding the arrow in the opposite direction of the selection, setting it to the start of the selection
                if (input->nbCharSelected > 0) input->cursor.index = input->selectionStart;

                input->nbCharSelected = 0;

            }
            else {
                input->cursor.index = SDL_max((int)input->cursor.index - 1, 0);
            }
        }
        resetCursorBlink(app);
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);

        break;
    case SDLK_RIGHT:
        if (event->key.mod & SDL_KMOD_SHIFT) {
            if (input->nbCharSelected == 0) input->selectionStart = input->cursor.index;
            if (input->selectionStart + input->nbCharSelected < input->text.count) input->nbCharSelected++;
            input->cursor.index = SDL_min(input->cursor.index + 1, input->text.count);
        }
        else {
            // If shift has stopped being held and something is selected unselect
            // it and set the cursor position to the start of the selection
            if (input->nbCharSelected != 0) {
                // We are holding the arrow in the opposite direction of the selection, setting it to the start of the selection
                if (input->nbCharSelected < 0) input->cursor.index = input->selectionStart;

                input->nbCharSelected = 0;
            }
            else {
                input->cursor.index = SDL_min(input->cursor.index + 1, input->text.count);
            }
        }
        resetCursorBlink(app);
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
        break;
    default: break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult appendTextToTextInputOnTextInputEvent(SDL_Event* event, App* app) {
    if (app->events.textInput.isActive) {
        appendTextToTextInput(app, event->text.text);
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult clickToCursor(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

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

    input->cursor.index = newIndex;
    resetCursorBlink(app);
    SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    return SDL_APP_CONTINUE;
}

SDL_AppResult changeMouseIconOnEnterTextInput(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    app->events.textInput.cursor.sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    resetCursorBlink(app);
    SDL_SetCursor(app->events.textInput.cursor.sdlCursor);

    return SDL_APP_CONTINUE;
}

SDL_AppResult resetMouseIconOnExitTextInput(SDL_Event* event, SDL_FRect rect, App* app) {
    (void)event, (void)rect;

    SDL_DestroyCursor(app->events.textInput.cursor.sdlCursor);
    app->events.textInput.cursor.sdlCursor = SDL_GetDefaultCursor();
    SDL_SetCursor(app->events.textInput.cursor.sdlCursor);

    return SDL_APP_CONTINUE;
}

SDL_AppResult resetTextInputSelectionOnMouseButtonDown(SDL_Event* event, SDL_FRect rect, App* app) {
    if (event->button.button == SDL_BUTTON_LEFT) {
        // Put the cursor's index at the correct position
        if (clickToCursor(event, rect, app) != SDL_APP_CONTINUE) return SDL_APP_FAILURE;
        // Reset the nbCharSelected and the selectionStart
        app->events.textInput.nbCharSelected = 0;
        app->events.textInput.selectionStart = app->events.textInput.cursor.index;
        SDL_SetAtomicInt(&app->state.currentScene.shouldRender, MAIN_THREAD_RERENDER);
    }

    return SDL_APP_CONTINUE;
}
