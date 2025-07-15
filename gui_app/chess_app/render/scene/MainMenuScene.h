#ifndef DC6832D3_BA2C_425A_A3AA_925718E06AF6
#define DC6832D3_BA2C_425A_A3AA_925718E06AF6

#include "../../AppState.h"

typedef enum MainMenuRenderBoxIndex {
    WHITE_PLAYER_INFO,
    WHITE_SETTINGS,
    WHITE_TIME_CONTROL,
    BLACK_PLAYER_INFO,
    BLACK_SETTINGS,
    BLACK_TIME_CONTROL,
    STARTING_POSITION,
    START_GAME,
    MAIN_MENU_CREDITS,
    TOTAL_MAIN_MENU_RENDER_BOX
} MainMenuRenderBoxIndex;

/**
 * @brief Computes the SceneRender struct for the main menu scene based on the current 
 * window size
 * 
 * @param app The app's data pointer
 */
SDL_AppResult computeMainMenuSceneRender(App* app);

void terminateMainMenuScene(void* data);

#endif /* DC6832D3_BA2C_425A_A3AA_925718E06AF6 */
