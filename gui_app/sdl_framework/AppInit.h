#ifndef BC27FC8A_BD51_4933_9D9F_CD61A2488254
#define BC27FC8A_BD51_4933_9D9F_CD61A2488254

#include <stdbool.h>

#include "../../engine/src/utils/Types.h"

#include "State.h"

/**
 * @brief Initializes the sdl library, ttf engine and image library of sdl.
 *
 * @param sdlFlags The initialization flags for the sdl library
 *
 * @return true If the library were correctly initialized
 * @return false If the library could not be initialized
 */
bool initializeSDlLibraries(u32 sdlFlags);

/**
 * @brief Initializes the SDL_state struct. Requires the initializeSDlLibraries
 * function to be called before calling this one.
 *
 * @param sdlState The sdlState to initialized
 * @param windowTitle Window's title
 * @param windowX Window's x position
 * @param windowY Window's y position
 * @param windowWidth Window's width
 * @param windowHeight Window's height
 * @param windowFlags Window's initialization flags
 * @param rendererName The renderer's name or NULL if we want to let SDL decide
 * @param fontPath The path to the font file
 * @param fontSize The size of the font
 * @return true If the sdl state was correctly initialized
 * @return false If an error occurred
 */
bool initializeSDLState(SDL_State* sdlState,
    const char* windowTitle, int windowX, int windowY, int windowWidth, int windowHeight, u32 windowFlags,
    const char* rendererName,
    const char* fontPath, float fontSize);

/**
 * @brief Initializes the textures dynamic array
 *
 * @param textures The textures struct to initialize
 * @param initialCapacity The initial capacity of the textures
 * @return true If the textures are initialized correctly
 * @return false If the textures were not correctly initialized
 */
bool initializeTextures(Textures* textures, size_t initialCapacity);

/**
 * @brief Loads images into textures from file paths
 * 
 * @param sdlState The sdl states properly initialized
 * @param textures The textures struct properly initializes
 * @param filePaths The file path of the images
 * @param nbImages The number of images
 * @return true If the images were properly loaded as textures
 * @return false If an error occurred
 */
bool loadImageFromFilePath(SDL_State* sdlState, Textures* textures, const char** filePaths, size_t nbImages);

/**
 * @brief First function called when starting the app.
 * 
 * @param app The state and events of the app
 * @return true If everything was correctly initialized
 * @return false If an error occurred
 */
bool initializeApp(App* app);

#endif /* BC27FC8A_BD51_4933_9D9F_CD61A2488254 */
