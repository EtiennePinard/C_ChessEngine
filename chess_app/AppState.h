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
#define PLACEHOLDER_RECT ((SDL_Rect) { .x = PLACEHOLDER_X, .y = PLACEHOLDER_Y, .w = PLACEHOLDER_WIDTH, .h = PLACEHOLDER_HEIGHT })

#define CHESSBOARD_X (WINDOW_WIDTH / 3)
#define CHESSBOARD_Y (0)
#define CHESSBOARD_WIDTH (WINDOW_WIDTH * 2 / 3)
#define CHESSBOARD_HEIGHT (WINDOW_HEIGHT)
#define CHESSBOARD_RECT ((SDL_Rect) { .x = CHESSBOARD_X, .y = CHESSBOARD_Y, .w = CHESSBOARD_WIDTH, .h = CHESSBOARD_HEIGHT })

// Struct to hold application state
typedef struct SDL_State {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
} SDL_State;

typedef struct ImageData {
    SDL_Texture *texture;
    int width;
    int height;
} ImageData;

typedef struct DraggingState {
    bool isDragging;
    char from;
    Piece draggedPiece;
} DraggingState;

typedef enum GameResult {
    GAME_IS_NOT_DONE,
    THREE_MOVE_REPETITION,
    STALEMATE,
    FIFTY_MOVE_RULE,
    WHITE_WON_CHECKMATE,
    BLACK_WON_CHECKMATE,
    WHITE_WON_ON_TIME,
    BLACK_WON_ON_TIME
} GameResult;

typedef struct GameState {
    ChessGame currentState;
    ChessGame* previousStates;
    int previousStateCapacity;
    int previousStateIndex;

    GameResult result;
} GameState;

typedef struct ClickableArea {
    SDL_Rect rect;
    void (*callback)(SDL_Event, GameState*, DraggingState*);
} ClickableArea;

typedef struct ClickableAreas {
    ClickableArea* areas;
    size_t count;
    size_t capacity;
} ClickableAreas;

#define clickableAreas_append(clickableAreas, clickableArea) if (clickableAreas->count >= clickableAreas->capacity) { \
        clickableAreas->areas = realloc(clickableAreas->areas, sizeof(ClickableArea) * clickableAreas->capacity * 2); \
        clickableAreas->capacity *= 2; \
    } \
    clickableAreas->areas[clickableAreas->count++] = clickableArea; \

// Assuming that 0 <= indexToUpdate <= count
// Note: Since we have update, we cannot have a remove function, since it would offset the indices that 
// functions rely on to update the clickable area
#define clickableAreas_update(clickableAreas, indexToUpdate, newValue) clickableAreas->areas[indexToUpdate] = newValue;

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
