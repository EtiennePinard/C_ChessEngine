#ifndef C7341AE2_8B99_441E_A1B1_E6E078854516
#define C7341AE2_8B99_441E_A1B1_E6E078854516

#include "MainMenuScene.h"

typedef struct TimeControlModalData {
    TimeControl hovered;
    PieceCharacteristics playerColor;
} TimeControlModalData;

typedef struct EngineConfigModalData {
    EngineConfig currentConfig;
    PieceCharacteristics playerColor;

    SDL_FRect checkboxRect;
    SDL_FRect enginePathRect;
    SDL_FRect thinkTimeRect;
    SDL_FRect okButtonRect;
    SDL_FRect cancelButtonRect;
    SDL_FRect thinkByHimselfRect;

    bool wasTextInputExited;
} EngineConfigModalData;

typedef enum MainMenuModalId {
    TIME_CONTROL_MODAL_ID,
    ENGINE_CONFIG_MODAL_ID
} MainMenuModal;

void setTimeControlModalActive(App* app, PieceCharacteristics colorToSet);
void setEngineConfigModalActive(App* app, PieceCharacteristics colorToSet);

SDL_FRect computeMainMenuModalRect(SDL_Window* window);

#endif /* C7341AE2_8B99_441E_A1B1_E6E078854516 */
