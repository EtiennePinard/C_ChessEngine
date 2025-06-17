#ifndef AE539C76_7948_4E5B_AFD7_B75D43C5B451
#define AE539C76_7948_4E5B_AFD7_B75D43C5B451

#include "../AppState.h"

SDL_AppResult chessBoardMouseButtonDown(SDL_Event* event, SDL_Rect rect, App* app);
SDL_AppResult chessBoardMouseButtonUp(SDL_Event* event, SDL_Rect rect, App* app);

SDL_AppResult promotionOverlayMouseButtonDown(SDL_Event* event, SDL_Rect rect, App* app);

void clickedRestartButton(SDL_Event event, App app);
void clickedSwitchColorButton(SDL_Event event, App app);
void clickedBackButton(SDL_Event event, App app);
void clickedCopyFenButton(SDL_Event event, App app);

// Note: This will be correct if the point (x, y) is in the chessboard
static inline Square squareFromxy(int x, int y, bool flip, SDL_Rect chessBoardRect) {
    int squareSize = chessBoardRect.w / BOARD_LENGTH;
    int col = (x - chessBoardRect.x) / squareSize;
    int row = (y - chessBoardRect.y) / squareSize;
    if (flip) {
        col = BOARD_LENGTH - 1 - col;
        row = BOARD_LENGTH - 1 - row;
    }
    return (Square)row * BOARD_LENGTH + col;
}

#endif /* AE539C76_7948_4E5B_AFD7_B75D43C5B451 */
