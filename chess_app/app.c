#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "AppState.h"
#include "Render.h"
#include "AppInit.h"
#include "EventHandler.h"

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
    const char* initialFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    setupChesGame(&state.currentState, &state.currentState.currentPosition, initialFen);

    DraggingState draggingState = { 0 };

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                handleMouseButtonDown(&state, &draggingState);
                break;
            case SDL_MOUSEBUTTONUP:
                handleMouseButtonUp(&state, &draggingState);
                break;
            default:
                break;
            }
        }
        render(&sdlState, chessImages, &state, draggingState);
    }

    // Cleanup
    cleanupApp(&sdlState, chessImages);

    return 0;
}
