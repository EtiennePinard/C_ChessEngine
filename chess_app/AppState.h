#ifndef E50A5778_B8CC_4205_8BEF_5FD650592497
#define E50A5778_B8CC_4205_8BEF_5FD650592497

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../src/state/GameState.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define FONT_SIZE 24

#define PLACEHOLDER_X (0)
#define PLACEHOLDER_Y (0)
#define PLACEHOLDER_WIDTH (WINDOW_WIDTH / 3)
#define PLACEHOLDER_HEIGHT (WINDOW_HEIGHT)

#define CHESSBOARD_X (WINDOW_WIDTH / 3)
#define CHESSBOARD_Y (0)
#define CHESSBOARD_WIDTH (WINDOW_WIDTH * 2 / 3)
#define CHESSBOARD_HEIGHT (WINDOW_HEIGHT)

// Struct to hold application state
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
} SDL_State;

typedef struct ImageData {
    SDL_Texture *texture;
    int width;
    int height;
} ImageData;

typedef struct GameState {
    ChessGame currentState;
    ChessGame* previousStates;
    int previousStateCapacity;
    int previousStateIndex;
} GameState;

typedef struct DraggingState {
    bool isDragging;
    char from;
    Piece draggedPiece;
} DraggingState;

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
