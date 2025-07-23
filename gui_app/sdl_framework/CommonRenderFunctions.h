#ifndef EB87E2EB_70FD_4BED_BC65_62901812984A
#define EB87E2EB_70FD_4BED_BC65_62901812984A

#include "State.h"

/**
 * @brief Finds the best font size so that the text takes the most 
 * amount of space inside the rectangle.
 * 
 * @param textString The text to draw
 * @param rectToFit The rectangle to fit the text into
 * @param baseFont The font to find the best size
 * @param isTextMultiLine If the text is multiline or not
 * @param resultingFontSize The pointer where the best font size to fit will be stored.
 * If this function returns SDL_APP_FAILURE this pointer will not be set and is 
 * left uninitialized
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult findFontSizeToFit(const char* textString, SDL_FRect rectToFit, TTF_Font* baseFont, bool isTextMultiLine, float* resultingFontSize);

/**
 * @brief Renders multiline text center justified and fitted using findFontSizeToFit
 * inside a rectangle.
 * 
 * @param renderer The renderer to draw the text to
 * @param font The font to render the text with. 
 * @param text The text to render
 * @param color The color of the text
 * @param rectToFit The rectangle to fit the text into
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderMultilineTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rectToFit);

/**
 * @brief Renders single line text centered and fitted using findFontSizeToFit
 * inside a rectangle.
 * 
 * @param renderer The renderer to draw the text to
 * @param font The font to render the text with. 
 * @param text The text to render
 * @param color The color of the text
 * @param rectToFit The rectangle to fit the text into
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderSingleLineTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rectToFit);

/**
 * @brief Renders the text of an active text input centered in a rectangle with
 * the correct cursor position and text highlighting
 * 
 * @param rect The rectangle to render the text into
 * @param app The app's data
 * @param hoverColor The color to fill the rect when the mouse hovers the text input
 * @param unselectedTextColor The color of unselected text
 * @param selectedTextColor The color of selected text
 * @param selectedTextBgColor The background color of selected text
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderTextInputCenteredToFit(SDL_FRect rect, App* app, SDL_Color hoverColor, SDL_Color unselectedTextColor, SDL_Color selectedTextColor, SDL_Color selectedTextBgColor);

/**
 * @brief Renders a labeled button to the window. Note that for
 * highlighting to function the scene must be rerender at least on
 * entering and exiting the button.
 * 
 * @param rect The rectangle to render the button to
 * @param app The app's data
 * @param hoverIndex The index of the render box this button is associated to.
 * This will be used for a hovering effect.
 * @param text The text to be displayed in the button
 * @param hoverColor The color to fill the rect when the mouse hovers the button
 * @param textColor The color of the text of the labeled button
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderButton(SDL_FRect rect, App* app, int hoverIndex, const char* text, SDL_Color hoverColor, SDL_Color textColor);

/**
 * @brief Renders a labeled checkbox to the window. Note that for
 * highlighting to function the scene must be rerender at least on
 * entering and exiting the button.
 * 
 * @param rect The rect to contain the checkbox and the label
 * @param app The app's data
 * @param checked If the checkbox is checked
 * @param label The checkbox's label
 * @param hoverIndex The index of the render box this button is associated to.
 * This will be used for a hovering effect.
 * @param borderColor The color of the checkbox border
 * @param hoverColor The color to draw the border of the checkbox when the mouse hovers the button
 * @param checkedColor The color of the checkbox when checked is true
 * @param textColor The color of the text of the labeled button
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult renderLabeledCheckboxButton(SDL_FRect rect, App* app, bool checked, const char* label, int hoveredIndex,
    SDL_Color borderColor, SDL_Color hoverColor, SDL_Color checkedColor, SDL_Color textColor);

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


#endif /* EB87E2EB_70FD_4BED_BC65_62901812984A */
