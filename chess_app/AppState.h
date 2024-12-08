#ifndef E50A5778_B8CC_4205_8BEF_5FD650592497
#define E50A5778_B8CC_4205_8BEF_5FD650592497

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../src/state/GameState.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define FONT_SIZE 24
#define FONT_PATH ("./assets/font/cmunbl.ttf")
#define CHESS_IMAGE_BASE_PATH ("./assets/png")
#define TITLE ("Chess")


#define INITIAL_FEN ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
#define TIME_CONTROL_MS ((u32) (3 * 60 * 1000))

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

typedef struct SDL_State {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
} SDL_State;

typedef struct TextureState {
    SDL_Texture *texture;
    int width;
    int height;
} TextureState;

typedef struct Textures {
    TextureState *data;
    size_t count;
    size_t capacity;
} Textures;

typedef struct DraggingState {
    bool isDragging;
    char from;
    char to; // The square that it ended on
    Piece draggedPiece;
} DraggingState;

typedef enum GameResult {
    GAME_IS_NOT_DONE,
    THREE_MOVE_REPETITION,
    STALEMATE,
    INSUFFICIENT_MATERIAL,
    FIFTY_MOVE_RULE,
    WHITE_WON_CHECKMATE,
    BLACK_WON_CHECKMATE,
    WHITE_WON_ON_TIME,
    BLACK_WON_ON_TIME
} GameResult;

typedef struct GameState {
    ChessGame currentState;
    PieceCharacteristics playerColor;
    u64 turnStartTick;

    ChessGame* previousStates;
    int previousStateCapacity;
    int previousStateIndex;

    GameResult result;
} GameState;

typedef struct AppState {
    bool isRunning;
    SDL_State sdlState;
    Textures textures;
    GameState gameState;
    DraggingState draggingState;
} AppState;

typedef struct Popup {
    SDL_Rect rect;
    // Returns true if the popup worked, else returns false
    bool (*callback)(SDL_Event, SDL_Rect, AppState*);
} Popup;

typedef struct ClickableArea {
    SDL_Rect rect;
    void (*callback)(SDL_Event, AppState*);
} ClickableArea;

typedef struct ClickableAreas {
    ClickableArea* data;
    size_t count;
    size_t capacity;
} ClickableAreas;

typedef struct AppEvents {
    ClickableAreas clickableAreas;
} AppEvents;

#define da_append(da, valueToAppend) if (da->count >= da->capacity) { \
        da->data = realloc(da->data, sizeof(valueToAppend) * da->capacity * 2); \
        da->capacity *= 2; \
    } \
    da->data[da->count++] = valueToAppend; \

// Assuming that 0 <= indexToUpdate <= count
// Note: Since we have update, we cannot have a remove function, since it would offset the indices that 
// functions rely on to update the clickable area
#define da_update(da, indexToUpdate, newValue) da->data[indexToUpdate] = newValue;

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
