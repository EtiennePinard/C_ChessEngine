#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../AppStyle.h"
#include "RenderUtils.h"

SDL_AppResult renderMultilineTextCentered(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_Rect rect) {
    if (!renderer || !font || !text) return SDL_APP_FAILURE;

    char* textCopy = strdup(text);
    if (!textCopy) return SDL_APP_FAILURE;

    int lineHeight = TTF_GetFontLineSkip(font);
    int numLines = 0;

    // Count lines
    for (const char* p = text; *p; p++) {
        if (*p == '\n') numLines++;
    }
    numLines++; // at least one line

    int totalTextHeight = numLines * lineHeight;
    int y = rect.y + (rect.h - totalTextHeight) / 2;

    // Tokenize and render each line
    char* saveptr = NULL;
    char* line = strtok_r(textCopy, "\n", &saveptr);
    while (line) {
        SDL_Surface* surf = TTF_RenderText_Blended(font, line, 0, color);
        if (!surf) {
            free(textCopy);
            return SDL_APP_FAILURE;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (!tex) {
            SDL_DestroySurface(surf);
            free(textCopy);
            return SDL_APP_FAILURE;
        }

        SDL_Rect dst = {
            .x = rect.x + (rect.w - surf->w) / 2,
            .y = y,
            .w = surf->w,
            .h = surf->h
        };

        SDL_RenderTexture(renderer, tex, NULL, &RECT_TO_FRECT(dst));

        y += lineHeight;

        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
        line = strtok_r(NULL, "\n", &saveptr);
    }

    free(textCopy);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderCenteredSingleLineText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_Rect rect) {
    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text, 0, color, 0);
    if (!textSurface) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_RenderText_Blended failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
        SDL_DestroySurface(textSurface);
        return SDL_APP_FAILURE;
    }

    // Centering the text from the baseline
    int ascent = TTF_GetFontAscent(font);
    SDL_Rect textRect = {
    .x = rect.x + (rect.w - textSurface->w) / 2,
    .y = rect.y + (rect.h - ascent) / 2,
    .w = textSurface->w,
    .h = textSurface->h
    };
    SDL_RenderTexture(renderer, textTexture, NULL, &RECT_TO_FRECT(textRect));
    SDL_DestroySurface(textSurface);
    SDL_DestroyTexture(textTexture);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* baseFont, const char* textString, bool isTextMultiLine, SDL_Color color, SDL_Rect rect) {
    if (!renderer || !baseFont || !textString) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter at " __FILE__);
        return SDL_APP_FAILURE;
    }

    // Initial guess font size
    const float referenceSize = 16.0;

    // Copy and set base font to reference size
    TTF_Font* tempFont = TTF_CopyFont(baseFont);
    if (!tempFont) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_CopyFont failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_SetFontSize(tempFont, referenceSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_SetFontSize(%f) failed: %s\n", referenceSize, SDL_GetError());
        goto end;
    }

    int textW = 0, textH = 0;
    bool sizeSuccess = false;
    if (isTextMultiLine) sizeSuccess = TTF_GetStringSizeWrapped(tempFont, textString, 0, 0, &textW, &textH);
    else sizeSuccess = TTF_GetStringSize(tempFont, textString, 0, &textW, &textH);

    if (!sizeSuccess || textW == 0 || textH == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_GetTextSize failed: %s\n", SDL_GetError());
        goto end;
    }

    // Compute scale factor based on rect size
    float scaleW = (float)rect.w / (float)textW;
    float scaleH = (float)rect.h / (float)textH;
    float scale = fminf(scaleW, scaleH);

    float finalSize = referenceSize * scale;
    finalSize = SDL_clamp(finalSize, 4, rect.h);  // Prevent tiny or huge font sizes

    // Set the computed best size
    if (!TTF_SetFontSize(tempFont, (float)finalSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_SetFontSize(%f) failed: %s\n", finalSize, SDL_GetError());
        goto end;
    }

    // Drawing the text centered
    SDL_AppResult result;
    if (isTextMultiLine) {
        result = renderMultilineTextCentered(renderer, tempFont, textString, color, rect);
    } else {
        result = renderCenteredSingleLineText(renderer, tempFont, textString, color, rect);
    }

end:
    TTF_CloseFont(tempFont);
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

SDL_AppResult renderButton(SDL_Rect rect, App* app, int hoverIndex, const char* text) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color hightLightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;

    // If this is the currently hovered box, highlight it
    if (app->events.mouseState.hoveredIndex == hoverIndex) {
        SDL_SetRenderDrawColor(renderer, hightLightColor.r, hightLightColor.g, hightLightColor.b, hightLightColor.a);
        SDL_RenderFillRect(renderer, &RECT_TO_FRECT(rect));
    }

    // Render button text
    return renderTextCenteredToFit(renderer, font, text, false, textColor, rect);
}

SDL_AppResult renderCredits(SDL_Rect rect, App* app) {
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, false, CREDIT_COLOR, rect);
}
