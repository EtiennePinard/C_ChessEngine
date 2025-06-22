#ifndef B4B2C873_427F_42CE_9217_0E2D736F783C
#define B4B2C873_427F_42CE_9217_0E2D736F783C

#include "../AppState.h"

typedef enum GameSceneRenderBoxIndex {
    CHESSBOARD,
    BLACK_CLOCK,
    WHITE_CLOCK,
    MOVE_LIST,
    RESTART_BUTTON,
    BACK_BUTTON,
    PROMOTION_OVERLAY,
    GAME_ENDED_OVERLAY,
    TOTAL_GAME_SCENE_RENDER_BOX
} GameSceneRenderBoxIndex;

/**
 * @brief Computes the SceneRender struct for the game scene based on the current 
 * window size
 * 
 * @param window The window the scene will be drawn onto 
 * @param scene The scene object to fill the SceneRender struct of.
 */
void computeGameSceneRender(SDL_Window* window, Scene* scene);

void terminateGameScene(void* data);

#endif /* B4B2C873_427F_42CE_9217_0E2D736F783C */
