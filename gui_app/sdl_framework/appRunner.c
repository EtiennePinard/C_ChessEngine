#include "AppInit.h"
#include "EventHandler.h"
#include "Render.h"
#include "AppCleanup.h"

#include "AppRunner.h"

bool runApp(App app) {
    if (!initializeApp(app)) return false;

    while (!app.events->hasQuitEventHappened) {
        handleEvent(app);
        render(app);
    }

    cleanupApp(app);
    return true;
}
