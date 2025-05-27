#ifndef E50A5778_B8CC_4205_8BEF_5FD650592497
#define E50A5778_B8CC_4205_8BEF_5FD650592497

#include "../sdl_framework/State.h"

#include "../../engine/src/state/ChessPosition.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define FONT_SIZE 24
#define FONT_PATH ("./assets/font/cmunbl.ttf")
#define TITLE ("Chess")

#define STARTING_TIME_MS ((TimeControl_MS) (3 * 60 * 1000))

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

// Constants for clickable areas
typedef enum ClickableAreaIndex {
    CHESSBOARD_INDEX,
    RESTART_BUTTON_INDEX,
    SWITCH_BUTTON_INDEX,
    BACK_BUTTON_INDEX,
    COPY_FEN_BUTTON_INDEX,
    TOTAL_CLICKABLE_AREA
} ClickableAreaIndex;

typedef struct DraggingState {
    bool isDragging;
    Square from;
    Square to; // The square that it ended on
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

typedef u32 TimeControl_MS;

typedef struct UndoGameStates {
    ChessPosition* previousStates;
    int previousStateCapacity;
    int previousStateIndex;
} UndoGameStates;

typedef struct GameState {
    ChessPosition currentPosition;
    PieceCharacteristics playerColor;
    u64 turnStartTick;
    TimeControl_MS whiteRemainingTime;
    TimeControl_MS blackRemainingTime;

    UndoGameStates undoStates;
    Move* movesPlayed;

    GameResult result;
} GameState;

struct AppState {
    SDL_State sdlState;
    Textures textures;
    GameState gameState;
    DraggingState draggingState;
};

#endif /* E50A5778_B8CC_4205_8BEF_5FD650592497 */
