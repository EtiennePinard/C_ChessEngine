#ifndef D3396E27_D64F_4822_A2B1_53F51ECF3977
#define D3396E27_D64F_4822_A2B1_53F51ECF3977

#include "State.h"

/**
 * @brief Called every frame and properly handles all events in the AppEvents struct.
 *
 * @param app The state and events of the app
 */
void handleEvent(App app);

/**
 * @brief Call this function manually to run a popup. The standard way to create popups is this:
 *
 * @code{.c}
 * Popup popup = { 0 };
 * popup.callback = &popupCallback;
 * renderPopup(...);
 * handlePopup(&popup, appState);
 * @endcode
 *
 * The only events which will be handled when this function is called are the
 * quit event for the app and the mouse up and mouse down event for the popup.
 * This means that if a click is made outside the popup in a clickable area
 * the callback of this clickable area will not be ran.
 *
 * @param popup The popup which the callback will be called
 * @param app The state and events of the app
 */
void handlePopup(Popup* popup, App app);

#endif /* D3396E27_D64F_4822_A2B1_53F51ECF3977 */
