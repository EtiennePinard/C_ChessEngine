#include <SDL2/SDL_image.h>

#include "AppInit.h"
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/state/ZobristKey.h"
#include "../src/chessBot/PieceSquareTable.h"

static const char* PIECE_NAMES[NB_PIECE_COLOR][NB_PIECE_TYPE] = {
    {"white_pawn", "white_knight", "white_bishop", "white_rook", "white_queen", "white_king"},
    {"black_pawn", "black_knight", "black_bishop", "black_rook", "black_queen", "black_king"}
};

static bool loadChessImages(SDL_State* sdlState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], const char* basePath) {
    char filePath[256];
    for (PieceCharacteristics type = PAWN; type <= KING; type++) {
        for (PieceCharacteristics color = WHITE; color <= BLACK; color += WHITE) {
            snprintf(filePath, sizeof(filePath), "%s/%s.png", basePath, PIECE_NAMES[color / WHITE - 1][type - 1]);
            // Load the SVG as a texture
            SDL_RWops* rw = SDL_RWFromFile(filePath, "rb");
            SDL_Surface* surface = IMG_Load_RW(rw, 1);
            if (!surface) {
                fprintf(stderr, "Failed to load image %s: %s\n", filePath, SDL_GetError());
                return false;
            }

            chessImages[color / WHITE - 1][type - 1].texture = SDL_CreateTextureFromSurface(sdlState->renderer, surface);
            if (!chessImages[color / WHITE - 1][type - 1].texture) {
                fprintf(stderr, "Failed to create texture for %s: %s\n", filePath, SDL_GetError());
                SDL_FreeSurface(surface);
                return false;
            }

            chessImages[color / WHITE - 1][type - 1].width = surface->w;
            chessImages[color / WHITE - 1][type - 1].height = surface->h;
            SDL_FreeSurface(surface);
        }
    }
    return true;
}

bool initializeApp(SDL_State *sdlState, 
    const char* title, int width, int height, 
    const char* fontPath, int fontSize, 
    const char* svgBasePath, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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

    sdlState->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!sdlState->window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->renderer = SDL_CreateRenderer(sdlState->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlState->renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    sdlState->font = TTF_OpenFont(fontPath, fontSize);
    if (!sdlState->font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(sdlState->renderer);
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if (!loadChessImages(sdlState, chessImages, svgBasePath)) {
        fprintf(stderr, "Failed to load images.\n");
        SDL_DestroyRenderer(sdlState->renderer);
        SDL_DestroyWindow(sdlState->window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    magicBitBoardInitialize();
    zobristKeyInitialize();
    pieceSquareTableInitialize();

    return true;
}

static void freeChessImages(ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE]) {
    for (PieceCharacteristics type = PAWN; type <= KING; type++) {
        for (PieceCharacteristics color = WHITE; color <= BLACK; color += WHITE) {
            if (chessImages[color / WHITE - 1][type - 1].texture) {
                SDL_DestroyTexture(chessImages[color / WHITE - 1][type - 1].texture);
                chessImages[color / WHITE - 1][type - 1].texture = NULL;
            }
        }
    }
}

void cleanupApp(SDL_State *sldState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], GameState *gameState, ClickableAreas *clickableAreas) {
    magicBitBoardTerminate();
    freeChessImages(chessImages);
    free(gameState->previousStates);
    free(clickableAreas->areas);
    if (sldState->font) {
        TTF_CloseFont(sldState->font);
    }
    if (sldState->renderer) {
        SDL_DestroyRenderer(sldState->renderer);
    }
    if (sldState->window) {
        SDL_DestroyWindow(sldState->window);
    }
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();

}
