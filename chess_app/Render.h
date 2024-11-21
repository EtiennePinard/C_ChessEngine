#ifndef A9841FAA_4E77_45B1_BB21_63F917E2166B
#define A9841FAA_4E77_45B1_BB21_63F917E2166B

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "AppState.h"
#include "../src/utils/Utils.h"
#include "../src/state/Piece.h"
#include "../src/state/GameState.h"

void render(SDL_State* sdlState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], GameState* gameState, DraggingState draggingState);

#endif /* A9841FAA_4E77_45B1_BB21_63F917E2166B */
