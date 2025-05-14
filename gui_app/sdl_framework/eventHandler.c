#include <SDL2/SDL.h>

#include "EventHandler.h"

static bool pointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

void handlePopup(Popup* popup, App app) {
    // If the popup is active, only register clicks in the popup area
    SDL_Event event;
    int mouseX, mouseY;
    bool isActive = true;
    while (isActive) {

        while (SDL_PollEvent(&event)) {

            switch (event.type) {
            case SDL_QUIT:
                app.events->hasQuitEventHappened = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&mouseX, &mouseY);
                if (pointInRect(mouseX, mouseY, popup->rect)) {
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
    int mouseX, mouseY;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_QUIT:
            app.events->hasQuitEventHappened = true;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            SDL_GetMouseState(&mouseX, &mouseY);

            for (size_t index = 0; index < app.events->clickableAreas.capacity; index++) {
                ClickableArea area = app.events->clickableAreas.data[index];
                if (area.callback != NULL && pointInRect(mouseX, mouseY, area.rect)) {
                    area.callback(event, app);
                }
            }
            break;
        default:
            break;
        }

    }
}
