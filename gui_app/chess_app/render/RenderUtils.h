#ifndef DDB8F9F1_B4AC_462A_883D_545E82BB7F71
#define DDB8F9F1_B4AC_462A_883D_545E82BB7F71

#include "../AppState.h"

/**
 * @brief Renders text to fit a rectangle
 *
 * @param renderer The renderer to draw the text to
 * @param font The font to use for drawing the text
 * @param text The text to draw
 * @param isTextMultiLine If the text is a multi-line string
 * @param color The color of the text
 * @param rect The rectangle to fit the text into
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* font, const char* text, bool isTextMultiLine, SDL_Color color, SDL_Rect rect);

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
 * @brief Renders a button to the window
 * 
 * @param rect The rectangle to render the button to
 * @param app The app's data
 * @param hoverIndex The index of the render box this button is associated to.
 * This will be used for a hovering effect.
 * @param text The text to be displayed in the button
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderButton(SDL_Rect rect, App* app, int hoverIndex, const char* text);

/**
 * @brief Draws a filled circle
 * 
 * @param renderer The rendering target
 * @param cx The circle x offset
 * @param cy The circle y offset
 * @param radius The radius of the circle
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius);

/**
 * @brief The render function for the credits of the app
 * 
 * @param rect The rectangle to draw the credits in
 * @param app The data associated with this app
 * @return SDL_AppResult If the app should continue or terminate with failure or success 
 */
SDL_AppResult renderCredits(SDL_Rect rect, App* app);

#endif /* DDB8F9F1_B4AC_462A_883D_545E82BB7F71 */
