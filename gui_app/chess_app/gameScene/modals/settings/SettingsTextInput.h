#ifndef D91C1B20_BC58_431E_84D9_03F433BBB0C5
#define D91C1B20_BC58_431E_84D9_03F433BBB0C5

#include "../GameModals.h"

typedef enum SettingsTextInputId {
    STARTING_POSITION_TEXT_INPUT_ID,
    THINK_TIME_TEXT_INPUT_ID
} SettingsTextInputId;

void setStartingPositionTextInputActive(SDL_FRect rect, App* app);
void setTimeToThinkTextInputActive(SDL_FRect rect, App* app);

#endif /* D91C1B20_BC58_431E_84D9_03F433BBB0C5 */
