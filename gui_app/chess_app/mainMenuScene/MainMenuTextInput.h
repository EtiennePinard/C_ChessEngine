#ifndef CFE93147_8C8D_4DE3_9830_7E2E66147792
#define CFE93147_8C8D_4DE3_9830_7E2E66147792

#include "MainMenuScene.h"

typedef enum MainMenuTextInputId {
    STARTING_POSITION_TEXT_INPUT_ID,
    THINK_TIME_TEXT_INPUT_ID
} MainMenuTextInputId;

void setStartingPositionTextInputActive(SDL_FRect rect, App* app);
void setTimeToThinkTextInputActive(SDL_FRect rect, App* app);

#endif /* CFE93147_8C8D_4DE3_9830_7E2E66147792 */
