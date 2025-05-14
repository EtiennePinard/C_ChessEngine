#ifndef B09ED3F7_CFED_48B6_B6E5_530CF168497E
#define B09ED3F7_CFED_48B6_B6E5_530CF168497E

#include "State.h"

/**
 * @brief Frees the elements in the textures struct
 * 
 * @param textures The texture struct to free the elements of
 */
void cleanupTextures(Textures textures);

/**
 * @brief Frees the memory associated with the clickable ares
 * 
 * @param events The events of the app which includes clickable areas
 */
void cleanupClickableAreas(AppEvents* events);

/**
 * @brief Properly closes the components in the SDL_State struct
 * 
 * @param sdlState The SDL_State to cleanup
 */
void cleanupSDL_State(SDL_State sdlState);

/**
 * @brief Quits the SDL library, font engine and image libraries.
 *
 */
void quitSDL();

/**
 * @brief Cleans up the app. This function is called after a quit event
 * has occurred.
 * 
 * @param app The app struct to free
 */
void cleanupApp(App app);

#endif /* B09ED3F7_CFED_48B6_B6E5_530CF168497E */
