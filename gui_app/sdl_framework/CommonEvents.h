#ifndef FBD4A449_142C_4636_9D14_3A98AB19D007
#define FBD4A449_142C_4636_9D14_3A98AB19D007

#include "State.h"

/**
 * @brief Events callbacks which simply sets the shouldRender atomic 
 * integer to MAIN_THREAD_RERENDER
 * 
 * @param event (UNUSED) The event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult Always returns SDL_APP_CONTINUE
 */
SDL_AppResult rerenderScene(SDL_Event* event, SDL_FRect rect, App *app);

/**
 * @brief Set the isActive field for the modal to false, frees its data, and sets the shouldRender atomic
 * integer to MAIN_THREAD_RERENDER. This function is of the type
 * EventCallbackBox so that it can be used in a RenderBox struct.
 * 
 * @param event (UNUSED) The event this callback is called from
 * @param rect (UNUSED) The render rectangle associated to the callback
 * @param app The app's data
 * @return SDL_AppResult Always returns SDL_APP_CONTINUE
 */
SDL_AppResult closeModalEventCallbackBox(SDL_Event* event, SDL_FRect rect, App* app);

/**
 * @brief Set the isActive field for the modal to false, frees its data, and sets the shouldRender atomic
 * integer to MAIN_THREAD_RERENDER. This function is of the type EventCallback 
 * so that it can be used with onEscape or onReturn struct fields.
 * 
 * @param event (UNUSED) The event this callback is called from
 * @param app The app's data
 * @return SDL_AppResult Always returns SDL_APP_CONTINUE
 */
SDL_AppResult closeModalEventCallback(SDL_Event* event, App* app);

/**
 * @brief Set the isActive field for the textinput to false, frees the text's data, 
 * calls SDL_StopTextInput, and sets the shouldRender atomic integer to MAIN_THREAD_RERENDER. 
 * This function is of the type EventCallback so that it can be used with onEscape 
 * or onReturn struct fields.
 * 
 * @param event (UNUSED) The event this callback is called from
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult closeTextInput(SDL_Event* event, App* app);

/**
 * @brief Sets the cursor index based on the mouse click.
 * 
 * @param app The app's data
 * @return SDL_AppResult If the app should continue or terminate with failure or success
 */
SDL_AppResult clickToCursor(App* app);

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

#endif /* FBD4A449_142C_4636_9D14_3A98AB19D007 */
