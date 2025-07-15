#ifndef B4B2C873_427F_42CE_9217_0E2D736F783C
#define B4B2C873_427F_42CE_9217_0E2D736F783C

#include "../../AppState.h"

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

#endif /* B4B2C873_427F_42CE_9217_0E2D736F783C */
