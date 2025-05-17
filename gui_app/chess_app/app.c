#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "../sdl_framework/AppRunner.h"
#include "../sdl_framework/AppInit.h"
#include "../sdl_framework/AppCleanup.h"

#include "../../engine/src/magicBitBoard/MagicBitBoard.h"
#include "../../engine/src/state/ZobristKey.h"
#include "../../engine/src/bot/PieceSquareTable.h"
#include "../../engine/src/bot/TranspositionTable.h"
#include "../../engine/src/utils/FenString.h"

#include "AppState.h"
#include "Events.h"

static const char* PIECE_NAMES[NB_PIECES] = {
    "./assets/png/white_pawn.png", "./assets/png/white_knight.png", "./assets/png/white_bishop.png", "./assets/png/white_rook.png", "./assets/png/white_queen.png", "./assets/png/white_king.png",
    "./assets/png/black_pawn.png", "./assets/png/black_knight.png", "./assets/png/black_bishop.png", "./assets/png/black_rook.png", "./assets/png/black_queen.png", "./assets/png/black_king.png"
};

bool initializeApp(App app) {
    if (!initializeSDlLibraries(SDL_INIT_VIDEO | SDL_INIT_TIMER, IMG_INIT_PNG) ||
        !initializeSDLState(&app.state->sdlState,
            TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
            FONT_PATH, FONT_SIZE) ||
        !initializeTextures(&app.state->textures) ||
        !initializeClickableArea(app.events, TOTAL_CLICKABLE_AREA) ||
        !loadImageFromFilePath(&app.state->sdlState, &app.state->textures, PIECE_NAMES, NB_PIECES)) {
        return false;
    }

    // Chessboard position will not change while the app is running, so we can add it once at app startup
    app.events->clickableAreas.data[CHESSBOARD_INDEX] = (ClickableArea){ .rect = CHESSBOARD_RECT, .callback = &clickedChessBoard };

    if (!FenString_setChessPositionFromCopiedFenString(INITIAL_FEN, &app.state->gameState.currentState)) {
        fprintf(stderr, "Failed to initialize the initial position\n");
        SDL_DestroyRenderer(app.state->sdlState.renderer);
        SDL_DestroyWindow(app.state->sdlState.window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    app.state->gameState.playerColor = app.state->gameState.currentState.colorToGo;
    app.state->gameState.whiteRemainingTime = STARTING_TIME_MS;
    app.state->gameState.blackRemainingTime = STARTING_TIME_MS;

    app.state->gameState.previousStates = malloc(sizeof(ChessPosition) * 64);
    app.state->gameState.previousStateCapacity = 64;
    app.state->gameState.previousStateIndex = 0;
    app.state->gameState.result = GAME_IS_NOT_DONE;


    app.state->draggingState.isDragging = false;

    MagicBitBoard_init();
    ZobristKey_init();
    PieceSquareTable_init();
    TranspositionTable_init();

    // We are officially running the app!
    app.events->hasQuitEventHappened = false;

    // We want to minimize the time that the player lose because of initialization
    // I know it is pretty negligible, but that doesn't mean we can't try
    app.state->gameState.turnStartTick = SDL_GetTicks64();
    return true;
}

void cleanupApp(App app) {
    MagicBitBoard_terminate();
    TranspositionTable_terminate();

    cleanupTextures(app.state->textures);

    free(app.state->gameState.previousStates);

    cleanupClickableAreas(app.events);
    cleanupSDL_State(app.state->sdlState);
    quitSDL();
}

// TODO: Add a way to go back in moves
// TODO: Add sounds Note: Upgrade to SDL3 before doing audio
int main() {
    AppState appState = { 0 };
    AppEvents appEvents = { 0 };
    App app = { .events = &appEvents, .state = &appState };

    if (!runApp(app)) {
        fprintf(stderr, "An error occurred while running the app.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
