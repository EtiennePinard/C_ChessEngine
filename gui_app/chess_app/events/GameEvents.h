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

/**
 * @brief Plays a move chosen by the bot. This function takes
 * a long time and so it creates a thread to compute the bot
 * move. The pointer to this thread is returned, which means
 * it is the caller's responsibility to either wait for the
 * thread or detach it, depending if it needs to use the value
 * of the bot's move immediately. Use SDL_WaitThread(thread, NULL)
 * to wait for the thread or SDL_DetachThread(thread) to detach the
 * thread.
 *
 * IMPORTANT: The thread will modify the GameState, which means
 * that if you don't wait for the thread you cannot modify
 * the gameState until this thread has finished since
 * it would create race conditions. To check if this
 * thread as finished from the gameState you can check if
 * its the player color to go. If it is the case, then
 * this function has finished executing.
 *
 * @param app The state of the app
 * @return SDL_Thread* The bot's thread or NULL if an error occurred
 */
SDL_Thread* playBotMove(App* app);

/**
 * @brief Resets the game state. Assumes that the textures and the initial position is already loaded
 * 
 * @param data The game scene data
 * @return SDL_AppResult If the app should continue or terminate with success or failure
 */
SDL_AppResult resetGame(GameSceneData* data);

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
