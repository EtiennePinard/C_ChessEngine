#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "AppState.h"
#include "Render.h"
#include "AppInit.h"
#include "EventHandler.h"

// TODO: Add a way to go back in moves
// TODO: Add sounds Note: Upgrade to SDL3 before doing audio
int main() {
    AppState appState = { 0 };
    AppEvents appEvents = { 0 };

    if (!initializeApp(&appEvents, &appState)) {
        fprintf(stderr, "Failed to initialize application.\n");
        exit(EXIT_FAILURE);
    }
    
    while (appState.isRunning) {
        handleEvent(&appEvents, &appState);
        render(&appEvents, &appState);
    }

    cleanupApp(&appEvents, &appState);

    return 0;
}
