#ifndef DDB8F9F1_B4AC_462A_883D_545E82BB7F71
#define DDB8F9F1_B4AC_462A_883D_545E82BB7

#include "AppState.h"

/**
 * @brief Formats the time in the min:sec format and writes it to a buffer
 *
 * @param milliseconds The milliseconds to format
 * @param output A buffer which contains the formatted time
 * @param outputSize The size of the buffer used for the output
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult formatTime(TimeControl_MS milliseconds, char* output, size_t outputSize);

/**
 * @brief Formats the time control in the "m:ss | s" format and writes it to a buffer
 *
 * @param timeControl The time control to format
 * @param output A buffer which contains the formatted time control
 * @param outputSize The size of the buffer used for the output
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult formatTimeControl(TimeControl timeControl, char* output, size_t outputSize);

/**
 * @brief The render function for the credits of the app
 * 
 * @param rect The rectangle to draw the credits in
 * @param app The data associated with this app
 * @return SDL_AppResult If the app should continue or terminate with failure or success 
 */
SDL_AppResult renderCredits(SDL_FRect rect, App* app);

#endif /* DDB8F9F1_B4AC_462A_883D_545E82BB7F71 */
