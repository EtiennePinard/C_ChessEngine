#ifndef AE539C76_7948_4E5B_AFD7_B75D43C5B451
#define AE539C76_7948_4E5B_AFD7_B75D43C5B451

#include "GameScene.h"

// Chessboard
SDL_AppResult chessBoardMouseButtonDown(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult chessBoardMouseButtonUp(SDL_Event* event, SDL_FRect rect, App* app);

// Promotion
SDL_AppResult clickedDownPromotionModal(SDL_Event* event, SDL_FRect rect, App* app);

// Buttons
SDL_AppResult clickedRestartButton(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult clickedDownFlipBoardButton(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult clickedSettingsButton(SDL_Event* event, SDL_FRect rect, App* app);

// Move list
SDL_AppResult clickedDownMoveList(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult movelistMouseWheelScrolled(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult clickedDownScrollbar(SDL_Event* event, SDL_FRect rect, App* app);

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
SDL_AppResult resetGame(App* app);

typedef enum LoadGameInfoResult {
    SUCCESS,
    INVALID_FEN_STRING,
    EMPTY_WHITE_ENGINE_PATH,
    INVALID_WHITE_ENGINE_PATH,
    EMPTY_BLACK_ENGINE_PATH,
    INVALID_BLACK_ENGINE_PATH
} LoadGameInfoResult;

/**
 * @brief Loads the gameInfo field from the GameSceneData struct
 * 
 * @param app The app state
 * @return LoadGameInfoResult The result of this operation
 */
LoadGameInfoResult loadGameInfo(App* app);

void playMoveOnBoard(GameSceneData* data, Move move);

/**
 * @brief Calculates the square index of the chessboard from the (x, y) sdl coordinates
 * Note: This will be correct if the point (x, y) is in the chessboard
 * 
 * @param x The x coordinates
 * @param y The y coordinates
 * @param flip If the board is currently flipper
 * @param chessBoardRect The SDL_FRect from which the board is renderer with
 * @return Square The square index which at the point (x, y) 
 */
static inline Square squareFromxy(float x, float y, bool flip, SDL_FRect chessBoardRect) {
    int squareSize = chessBoardRect.w / BOARD_LENGTH;
    int col = (x - chessBoardRect.x) / squareSize;
    int row = (y - chessBoardRect.y) / squareSize;
    if (flip) {
        col = BOARD_LENGTH - 1 - col;
        row = BOARD_LENGTH - 1 - row;
    }
    Square square = (Square)(row * BOARD_LENGTH + col);
    if (square < A8 || square > H1) return (Square)-1;
    else return square;
}

#endif /* AE539C76_7948_4E5B_AFD7_B75D43C5B451 */
