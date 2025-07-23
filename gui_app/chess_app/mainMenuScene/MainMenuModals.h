#ifndef C7341AE2_8B99_441E_A1B1_E6E078854516
#define C7341AE2_8B99_441E_A1B1_E6E078854516

#include "MainMenuScene.h"

typedef enum MainMenuModalId {
    TIME_CONTROL_MODAL_ID,
    ENGINE_CONFIG_MODAL_ID
} MainMenuModal;

void setTimeControlModalActive(App* app, PieceCharacteristics colorToSet);
void setEngineConfigModalActive(App* app, PieceCharacteristics colorToSet);

SDL_FRect computeMainMenuModalRect(SDL_Window* window);

#endif /* C7341AE2_8B99_441E_A1B1_E6E078854516 */
