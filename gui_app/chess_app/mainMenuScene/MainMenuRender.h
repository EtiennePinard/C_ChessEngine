#ifndef CBB35B3C_4D2D_4A27_B014_BD9BEF5D1F1D
#define CBB35B3C_4D2D_4A27_B014_BD9BEF5D1F1D

#include "MainMenuScene.h"

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

#endif /* CBB35B3C_4D2D_4A27_B014_BD9BEF5D1F1D */
