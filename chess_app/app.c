#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "AppState.h"
#include "Render.h"
#include "AppInit.h"
#include "EventHandler.h"

// TODO: Promotion. Note: will have to code the functionality of a popup, meaning that when the promotion overlay is active nothing else works
// TODO: Make the app compile and fix the buggy behavior of promotion cause it will happen
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
