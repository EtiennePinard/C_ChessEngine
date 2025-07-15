#ifndef FBD4A449_142C_4636_9D14_3A98AB19D007
#define FBD4A449_142C_4636_9D14_3A98AB19D007

#include "../AppState.h"

SDL_AppResult rerenderScene(SDL_Event* event, SDL_FRect rect, App *app);

SDL_AppResult closeModalEventCallbackBox(SDL_Event* event, SDL_FRect rect, App* app);
SDL_AppResult closeModalEventCallback(SDL_Event* event, App* app);

#endif /* FBD4A449_142C_4636_9D14_3A98AB19D007 */
