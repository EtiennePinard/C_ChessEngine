#ifndef F68006D8_3387_4849_AE06_31FA213E3DE3
#define F68006D8_3387_4849_AE06_31FA213E3DE3

#include "State.h"

/**
 * @brief Sets the cursor index based on the mouse click.
 *
 * @param event (UNUSED) The event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult clickToCursor(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Changes the mouse icon text input when the mouse enters the text input
 *
 * @param event (UNUSED) The event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult Always returns SDL_APP_CONTINUE
 */
SDL_AppResult changeMouseIconOnEnterTextInput(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Resets the mouse icon text input when the mouse exit the text input
 *
 * @param event (UNUSED) The event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult Always returns SDL_APP_CONTINUE
 */
SDL_AppResult resetMouseIconOnExitTextInput(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Uses clickToCursor to get the correct cursor position and then
 * resets the selected text.
 *
 * @param event The mouse button down event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult resetTextInputSelectionOnMouseButtonDown(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Updates the selected text correctly if the user moves the mouse
 * while holding left click
 *
 * @param event The mouse button down event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult updateTextInputSelectionOnMouseHovered(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Handles common text editing shortcuts with
 * the key down event. The shortcuts handled are:  
 * - left and right arrow to move the cursor  
 * - shift+left/right arrow to start/move the selection  
 * - ctrl+v to paste text  
 * - ctrl+c to copy text  
 *
 * @param event The mouse button down event this callback is called from
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult textInputKeyDown(SDL_Event* event, App* app);

/**
 * @brief Updates the selected text correctly if the user moves the mouse
 * while holding left click
 *
 * @param event The mouse button down event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult appendTextToTextInputOnTextInputEvent(SDL_Event* event, App* app);

#endif /* F68006D8_3387_4849_AE06_31FA213E3DE3 */
