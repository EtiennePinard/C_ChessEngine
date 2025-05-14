#ifndef B0FD67C6_C366_4FE6_91A9_F7B073A54863
#define B0FD67C6_C366_4FE6_91A9_F7B073A54863

#include "State.h"

/**
 * @brief Runs the SDL app. The code is a simple event loop
 * with four main function called.
 *
 *  1. initializeApp
 *
 *  2. handleEvent
 *
 *  3. render
 *
 *  4. cleanupApp
 *
 * You will need to implement the initializeApp, render and cleanupApp function
 * yourself. The handleEvent function already implements clickable areas and
 * popups functionalities.
 *
 * You will also need define the AppState struct. It is recommended to
 * have atleast the SDL_State struct in the AppState struct to make
 * rendering easier.
 *
 * The event loop will run while a quit event has not happened. The condition
 * relies on the hasQuitEventHappened field in the AppEvents struct.
 * The event loop will first call the handleEvent function and then call the render
 * function.
 *
 * @param app The state and events of the app
 * @return true If app ran correctly
 * @return false If an error occurred during initialization
 */
bool runApp(App app);

#endif /* B0FD67C6_C366_4FE6_91A9_F7B073A54863 */
