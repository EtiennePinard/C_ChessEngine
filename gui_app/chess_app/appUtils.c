#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../sdl_framework/CommonRenderFunctions.h"

#include "AppStyle.h"

#include "AppUtils.h"

char* copyString(const char* stringToCopy) {
    if (!stringToCopy) return NULL;
    size_t length = SDL_strlen(stringToCopy) + 1;
    char* result = SDL_calloc(length, sizeof(char));
    SDL_assert(SDL_strlcpy(result, stringToCopy, length) == length - 1);
    return result;
}

SDL_AppResult formatTime(TimeControl_MS milliseconds, char* output, size_t outputSize) {
    if (!output || outputSize < 6) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter or outputSize less than 6 at " __FILE__);
        return SDL_APP_FAILURE;
    }

    u32 totalSeconds = milliseconds / 1000;
    u32 minutes = totalSeconds / 60;
    u32 seconds = totalSeconds % 60;
    // Format the string as "mm:ss"
    snprintf(output, outputSize, "%02u:%02u", minutes, seconds);
    return SDL_APP_CONTINUE;
}

SDL_AppResult formatTimeControl(TimeControl timeControl, char* output, size_t outputSize) {
    if (!output || outputSize < 11) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter or outputSize less than 11 at " __FILE__);
        return SDL_APP_FAILURE;
    }

    u32 totalSeconds = timeControl.timeLeft / 1000;
    u32 minutesTL = totalSeconds / 60;
    u32 secondsTL = totalSeconds % 60;
    totalSeconds = timeControl.increment / 1000;
    u32 secondsIN = totalSeconds % 60;
    // Format the string as "m:ss + s"
    snprintf(output, outputSize, "%u:%02u + %u", minutesTL, secondsTL, secondsIN);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderCredits(SDL_FRect rect, App* app) {
    return renderSingleLineTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, CREDIT_COLOR, rect);
}
