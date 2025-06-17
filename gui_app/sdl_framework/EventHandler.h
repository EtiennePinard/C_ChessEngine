#ifndef D3396E27_D64F_4822_A2B1_53F51ECF3977
#define D3396E27_D64F_4822_A2B1_53F51ECF3977

#include "State.h"

/**
 * @brief Called every times an event happens.
 *
 * @param app The state and events of the app
 * @param event The event that was fired
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult handleEvent(App* app, SDL_Event* event);

#endif /* D3396E27_D64F_4822_A2B1_53F51ECF3977 */
