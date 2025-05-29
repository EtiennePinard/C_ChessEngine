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
#include "UCIEngineCommunication.h"

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

    // We simply need the move generation and repetition table to work for this bot
    if (!MagicBitBoard_init() || !ZobristKey_init()) {
        fprintf(stderr, "Failed to initialize the magic bit boards and/or Zobrist keys\n");
        return false;
    }

    if (!FenString_setChessPositionFromCopiedFenString(INITIAL_FEN, &app.state->gameState.currentPosition)) {
        fprintf(stderr, "Failed to initialize the initial position\n");
        return false;
    }
    
    if (!UCIEngine_initialize("./chessEngine")) {
        fprintf(stderr, "Failed to initialize the uci engine\n");
        return false;
    }
    
    app.state->gameState.playerColor = app.state->gameState.currentPosition.colorToGo;
    app.state->gameState.whiteRemainingTime = STARTING_TIME_MS;
    app.state->gameState.blackRemainingTime = STARTING_TIME_MS;
    app.state->gameState.blackIncrement = 0;
    app.state->gameState.whiteIncrement = 0;

    app.state->gameState.result = GAME_IS_NOT_DONE;
    
    app.state->draggingState.isDragging = false;
    
    app.state->gameState.undoStates.previousStateCapacity = 64;
    app.state->gameState.undoStates.previousStates = malloc(sizeof(ChessPosition) * app.state->gameState.undoStates.previousStateCapacity);
    app.state->gameState.undoStates.previousStateIndex = 0;
    
    app.state->gameState.movesPlayed = malloc(sizeof(Move) * app.state->gameState.undoStates.previousStateCapacity);
    
    // We are officially running the app!
    app.events->hasQuitEventHappened = false;

    // We want to minimize the time that the player lose because of initialization
    // I know it is pretty negligible, but that doesn't mean we can't try
    app.state->gameState.turnStartTick = SDL_GetTicks64();
    return true;
}

void cleanupApp(App app) {
    MagicBitBoard_terminate();
    UCIEngine_terminate();

    cleanupTextures(app.state->textures);

    free(app.state->gameState.undoStates.previousStates);

    cleanupClickableAreas(app.events);
    cleanupSDL_State(app.state->sdlState);
    quitSDL();
}

// TODO: Add sounds Note: Upgrade to SDL3 before doing audio
int main(int argc, char* argv[]) {
    // Supressing unused parameter warning
    (void) argc;
    (void) argv;

    AppState appState = { 0 };
    AppEvents appEvents = { 0 };
    App app = { .events = &appEvents, .state = &appState };

    if (!runApp(app)) {
        fprintf(stderr, "An error occurred while running the app.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
