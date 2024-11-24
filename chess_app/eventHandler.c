#include "EventHandler.h"
#include "Render.h"

static bool pointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

void handlePopup(Popup *popup, AppState *appState) {
    // If the popup is active, only register clicks in the popup area
    SDL_Event event;
    int mouseX, mouseY;
    bool isActive = true;
    while (isActive) {

        while (SDL_PollEvent(&event)) {

            switch (event.type) {
            case SDL_QUIT:
                appState->isRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&mouseX, &mouseY);
                if (pointInRect(mouseX, mouseY, popup->rect)) {
                    isActive = !popup->callback(event, popup->rect, appState);
                }
                break;
            default:
                break;
            }

        }

    }
}

void handleEvent(AppEvents *appEvents, AppState *appState) {
    SDL_Event event;
    int mouseX, mouseY;
    while (SDL_PollEvent(&event)) {

        switch (event.type) {
        case SDL_QUIT:
            appState->isRunning = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            SDL_GetMouseState(&mouseX, &mouseY);

            for (size_t index = 0; index < appEvents->clickableAreas.count; index++) {
                ClickableArea area = appEvents->clickableAreas.data[index];
                if (pointInRect(mouseX, mouseY, area.rect)) {
                    area.callback(event, appState);
                }
            }
            break;
        default:
            break;
        }

    }
}