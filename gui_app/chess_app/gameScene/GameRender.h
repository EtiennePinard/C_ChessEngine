#ifndef A258D4BE_7D1E_4C30_A4FF_3280B81AE3DB
#define A258D4BE_7D1E_4C30_A4FF_3280B81AE3DB

#include "GameScene.h"

typedef enum GameSceneRenderBoxIndex {
    CHESSBOARD,
    BLACK_CLOCK,
    WHITE_CLOCK,
    // It is very important that the scrollbar
    // renders before the move list else the scrolling
    // with the scrollbar will not work
    MOVE_LIST_SCROLLBAR,
    MOVE_LIST,
    RESTART_BUTTON,
    BACK_BUTTON,
    FLIP_BOARD_BUTTON,
    TOTAL_GAME_SCENE_RENDER_BOX
} GameSceneRenderBoxIndex;

/**
 * @brief Computes the SceneRender struct for the game scene based on the current 
 * window size
 * 
 * @param app The app's data pointer
 */
SDL_AppResult computeGameSceneRender(App* app);

void terminateGameScene(void* data);

#endif /* A258D4BE_7D1E_4C30_A4FF_3280B81AE3DB */
