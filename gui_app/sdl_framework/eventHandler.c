#include <SDL3/SDL.h>

#include "EventHandler.h"

static bool pointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

void handlePopup(Popup* popup, App app) {
    // If the popup is active, only register clicks in the popup area
    SDL_Event event;
    float mouseX, mouseY;
    bool isActive = true;
    while (isActive) {

        while (SDL_PollEvent(&event)) {

            switch (event.type) {
            case SDL_EVENT_QUIT:
                app.events->hasQuitEventHappened = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                SDL_GetMouseState(&mouseX, &mouseY);
                if (pointInRect((int) mouseX, (int) mouseY, popup->rect)) {
                    isActive = !popup->callback(event, popup->rect, app);
                }
                break;
            default:
                break;
            }

        }

    }
}

void handleEvent(App app) {
    SDL_Event event;
    float mouseX, mouseY;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_EVENT_QUIT:
            app.events->hasQuitEventHappened = true;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            SDL_GetMouseState(&mouseX, &mouseY);

            for (size_t index = 0; index < app.events->clickableAreas.capacity; index++) {
                ClickableArea area = app.events->clickableAreas.data[index];
                if (area.callback != NULL && pointInRect((int) mouseX, (int) mouseY, area.rect)) {
                    area.callback(event, app);
                }
            }
            break;
        default:
            break;
        }

    }
}
