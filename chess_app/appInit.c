#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include <stdio.h>

#include "AppInit.h"
#include "Events.h"
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/state/ZobristKey.h"
#include "../src/chessBot/PieceSquareTable.h"
#include "../src/utils/FenString.h"

static const char* PIECE_NAMES[NB_PIECE_COLOR][NB_PIECE_TYPE] = {
    {"white_pawn", "white_knight", "white_bishop", "white_rook", "white_queen", "white_king"},
    {"black_pawn", "black_knight", "black_bishop", "black_rook", "black_queen", "black_king"}
};

static bool loadChessImages(SDL_State* sdlState, Textures *chessImages, const char* basePath) {
    char filePath[256];
    // Note: It is important that the color goes first, because this is how the pieces are ordered in the Piece.h file
    for (PieceCharacteristics color = WHITE; color <= BLACK; color += WHITE) {
        for (PieceCharacteristics type = PAWN; type <= KING; type++) {

            snprintf(filePath, sizeof(filePath), "%s/%s.png", basePath, PIECE_NAMES[color / WHITE - 1][type - 1]);

            SDL_RWops* rw = SDL_RWFromFile(filePath, "rb");
            SDL_Surface* surface = IMG_Load_RW(rw, 1);
            if (!surface) {
                fprintf(stderr, "Failed to load image %s: %s\n", filePath, SDL_GetError());
                return false;
            }

            TextureState textureState = { 0 };

            textureState.texture = SDL_CreateTextureFromSurface(sdlState->renderer, surface);
            if (!textureState.texture) {
                fprintf(stderr, "Failed to create texture for %s: %s\n", filePath, SDL_GetError());
                SDL_FreeSurface(surface);
                return false;
            }

            textureState.width = surface->w;
            textureState.height = surface->h;
            SDL_FreeSurface(surface);

            da_append(chessImages, textureState);
        }
    }
    return true;
}

bool initializeApp(AppEvents *appEvents, AppState *appState) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Failed to initialize SDL_image: %s\n", IMG_GetError());
        return 1;
    }
    
    appState->sdlState.window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!appState->sdlState.window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    appState->sdlState.renderer = SDL_CreateRenderer(appState->sdlState.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!appState->sdlState.renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(appState->sdlState.window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    appState->sdlState.font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!appState->sdlState.font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(appState->sdlState.renderer);
        SDL_DestroyWindow(appState->sdlState.window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    appState->textures.capacity = 12; // Don't need more for now
    appState->textures.count = 0;
    appState->textures.data = malloc(sizeof(TextureState) * appState->textures.capacity);
    if (!loadChessImages(&appState->sdlState, &appState->textures, CHESS_IMAGE_BASE_PATH)) {
        fprintf(stderr, "Failed to load images\n");
        SDL_DestroyRenderer(appState->sdlState.renderer);
        SDL_DestroyWindow(appState->sdlState.window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if (!FenString_setChessPositionFromFenString(INITIAL_FEN, &appState->gameState)) {
        fprintf(stderr, "Failed to initialize the initial position\n");
        SDL_DestroyRenderer(appState->sdlState.renderer);
        SDL_DestroyWindow(appState->sdlState.window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    appState->gameState.playerColor = appState->gameState.currentState.colorToGo;
    appState->gameState.whiteRemainingTime = STARTING_TIME_MS;
    appState->gameState.blackRemainingTime = STARTING_TIME_MS;

    appState->gameState.previousStates = malloc(sizeof(ChessPosition) * 64);
    appState->gameState.previousStateCapacity = 64;
    appState->gameState.previousStateIndex = 0;
    appState->gameState.result = GAME_IS_NOT_DONE;


    appState->draggingState.isDragging = false;

    appEvents->clickableAreas.capacity = 2; // We don't have that much clickableAreas right now
    appEvents->clickableAreas.count = 0;
    appEvents->clickableAreas.data = malloc(sizeof(ClickableArea) * appEvents->clickableAreas.capacity);

    // Chessboard position will not change while the app is running, so we can add it once at app startup
    da_append((&appEvents->clickableAreas), ((ClickableArea) { .rect = CHESSBOARD_RECT, .callback = &clickedChessBoard}));

    MagicBitBoard_init();
    ZobristKey_init();
    PieceSquareTable_init();

    // We are officially running the app!
    appState->isRunning = true;

    // We want to minimize the time that the player lose because of initializatino
    // I know it is pretty negligeable, but that doesn't mean we can't try
    appState->gameState.turnStartTick = SDL_GetTicks64();
    return true;
}

static void freeTextures(Textures textures) {
    for (size_t index = 0; index < textures.capacity; index++) {
        if (textures.data[index].texture) {
            SDL_DestroyTexture(textures.data[index].texture);
            textures.data[index].texture = NULL;
        }
    }
}

void cleanupApp(AppEvents *appEvents, AppState *appState) {
    MagicBitBoard_terminate();
    freeTextures(appState->textures);

    free(appState->gameState.previousStates);
    free(appEvents->clickableAreas.data);
    if (appState->sdlState.font) {
        TTF_CloseFont(appState->sdlState.font);
    }
    if (appState->sdlState.renderer) {
        SDL_DestroyRenderer(appState->sdlState.renderer);
    }
    if (appState->sdlState.window) {
        SDL_DestroyWindow(appState->sdlState.window);
    }
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();

}
