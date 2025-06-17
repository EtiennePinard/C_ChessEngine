#ifndef DC6832D3_BA2C_425A_A3AA_925718E06AF6
#define DC6832D3_BA2C_425A_A3AA_925718E06AF6

#include "../AppState.h"

typedef enum MainMenuRenderBoxIndex {
    WHITE_KING_IMAGE,
    BLACK_KING_IMAGE,
    WHITE_PLAYER_TYPE,
    BLACK_PLAYER_TYPE,
    WHITE_ENGINE_PATH,
    BLACK_ENGINE_PATH,
    TIME_CONTROL_BUTTON,
    START_GAME,
    MAIN_MENU_CREDITS,
    TIME_CONTROL_MODAL,
    TOTAL_MAIN_MENU_RENDER_BOX
} MainMenuRenderBoxIndex;

/**
 * @brief Computes the SceneRender struct for the main menu scene based on the current 
 * window size
 * 
 * @param window The window the scene will be drawn onto 
 * @param sceneRender The sceneRender object to fill. 
 */
void computeMainMenuSceneRender(SDL_Window* window, SceneRender* sceneRender);

#endif /* DC6832D3_BA2C_425A_A3AA_925718E06AF6 */
