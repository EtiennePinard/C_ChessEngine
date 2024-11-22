#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "AppState.h"
#include "Render.h"
#include "AppInit.h"
#include "EventHandler.h"

// TODO: Promotion
int main() {
    SDL_State sdlState = { 0 };
    ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE] = { 0 };
    const char* imageBasePath = "./assets/png";
    const char* title = "SDL Chess Application";
    const char* fontPath = "/usr/share/fonts/truetype/computer-modern/cmunbl.ttf";

    if (!initializeApp(&sdlState, 
                        title, WINDOW_WIDTH, WINDOW_HEIGHT, 
                        fontPath, FONT_SIZE, 
                        imageBasePath, chessImages)) {
        fprintf(stderr, "Failed to initialize application.\n");
        exit(EXIT_FAILURE);
    }
    
    GameState state = { 0 };
    state.previousStates = malloc(sizeof(ChessGame) * 64);
    state.previousStateCapacity = 64;
    state.previousStateIndex = 0;
    state.result = GAME_IS_NOT_DONE;

    const char* initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    setupChesGame(&state.currentState, &state.currentState.currentPosition, initialFen);

    DraggingState draggingState = { 0 };

    ClickableAreas clickableAreas = { 0 };
    clickableAreas.capacity = 2; // We don't have that much clickableAreas right now
    clickableAreas.count = 0;
    clickableAreas.areas = malloc(sizeof(ClickableArea) * clickableAreas.capacity);

    // Chessboard position will not change while the app is running, so we can add it once at app startup
    clickableAreas_append((&clickableAreas), ((ClickableArea) { .rect = CHESSBOARD_RECT, .callback = &clickeChessBoard}));

    bool running = true;
    while (running) {
        handleEvent(&running, &clickableAreas, &state, &draggingState);
        render(&sdlState, chessImages, &state, draggingState, &clickableAreas);
    }

    cleanupApp(&sdlState, chessImages, &state, &clickableAreas);

    return 0;
}
