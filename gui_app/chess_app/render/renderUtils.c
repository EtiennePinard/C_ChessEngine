#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../AppStyle.h"
#include "RenderUtils.h"

SDL_AppResult renderMultilineTextCentered(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rect) {
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

        SDL_FRect dst = {
            .x = rect.x + (rect.w - surf->w) / 2,
            .y = y,
            .w = surf->w,
            .h = surf->h
        };

        SDL_RenderTexture(renderer, tex, NULL, &dst);

        y += lineHeight;

        SDL_DestroySurface(surf);
        SDL_DestroyTexture(tex);
        line = strtok_r(NULL, "\n", &saveptr);
    }

    free(textCopy);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderCenteredSingleLineText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rect, SDL_FRect* textRect) {
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
    SDL_FRect renderTextRect = {
    .x = rect.x + (rect.w - textSurface->w) / 2,
    .y = rect.y + (rect.h - ascent) / 2,
    .w = textSurface->w,
    .h = textSurface->h
    };
    SDL_RenderTexture(renderer, textTexture, NULL, &renderTextRect);
    SDL_DestroySurface(textSurface);
    SDL_DestroyTexture(textTexture);
    if (textRect) *textRect = renderTextRect;
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* baseFont, const char* textString, bool isTextMultiLine, SDL_Color color, SDL_FRect rect, SDL_FRect* textRect) {
    SDL_AppResult result = SDL_APP_FAILURE;

    if (!renderer || !baseFont || !textString) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "NULL parameter at " __FILE__);
        return result;
    }
    if (textString[0] == '\0') {
        // If we have an empty string simply return
        return SDL_APP_CONTINUE;
    }

    // Initial guess font size
    const float referenceSize = 16.0;

    // Copy and set base font to reference size
    TTF_Font* tempFont = TTF_CopyFont(baseFont);
    if (!tempFont) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_CopyFont failed: %s\n", SDL_GetError());
        return result;
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
    finalSize = SDL_clamp(finalSize, 4, rect.h); // Prevent tiny or huge font sizes

    // Set the computed best size
    if (!TTF_SetFontSize(tempFont, (float)finalSize)) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_SetFontSize(%f) failed: %s\n", finalSize, SDL_GetError());
        goto end;
    }

    // Drawing the text centered
    if (isTextMultiLine) result = renderMultilineTextCentered(renderer, tempFont, textString, color, rect);
    else result = renderCenteredSingleLineText(renderer, tempFont, textString, color, rect, textRect);

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

SDL_AppResult renderButton(SDL_FRect rect, App* app, int hoverIndex, const char* text) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color highlightColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color textColor = BUTTON_TEXT_COLOR;
    
    // If this is the currently hovered box, highlight it
    if (app->events.mouseState.hoveredIndex == hoverIndex &&
        SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &rect)) {
        SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
        SDL_RenderFillRect(renderer, &rect);
    }

    // Render button text
    return renderTextCenteredToFit(renderer, font, text, false, textColor, rect, NULL);
}

SDL_AppResult renderLabeledCheckboxButton(SDL_FRect rect, App* app, bool checked, const char* label, int hoveredIndex) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    SDL_Color borderColor = BUTTON_HIGHLIGHT_COLOR;
    SDL_Color highlightColor = BUTTON_BORDER_COLOR;
    SDL_Color fillColor = checked ? CHECKBOX_FILLED_COLOR : (SDL_Color) { 0, 0, 0, 0 };
    SDL_Color textColor = BUTTON_TEXT_COLOR;

    const float boxSize = rect.h * 0.8f;
    const float boxX = rect.x;
    const float boxY = rect.y + (rect.h - boxSize) / 2.0f;

    SDL_FRect checkboxRect = { boxX, boxY, boxSize, boxSize };

    // Draw border
    SDL_FPoint mousePoint;
    SDL_GetMouseState(&mousePoint.x, &mousePoint.y);
    if (app->events.mouseState.hoveredIndex == hoveredIndex &&
        SDL_PointInRectFloat(&mousePoint, &rect)) {
        // highlight the checkbox border if it is hovered
        SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
    }
    else {
        // else do not highlight it
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);

    }
    SDL_RenderRect(renderer, &checkboxRect);

    // Fill if checked
    if (checked) {
        SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
        SDL_RenderFillRect(renderer, &checkboxRect);
    }

    // Draw label to the right of the box
    SDL_FRect textRect = {
        .x = checkboxRect.x + checkboxRect.w + rect.h * 0.3f,
        .y = rect.y,
        .w = rect.w - checkboxRect.w - rect.h * 0.3f,
        .h = rect.h
    };

    return renderTextCenteredToFit(renderer, font, label, false, textColor, textRect, NULL);
}

SDL_AppResult drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius) {
    int maxPoints = (int)SDL_ceilf(4 * radius * radius) + 1; // + 1 for good measure, also PI = 4
    SDL_FPoint points[maxPoints];

    int count = 0;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                points[count].x = cx + dx;
                points[count].y = cy + dy;
                count++;
            }
        }
    }

    SDL_RenderPoints(renderer, points, count);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderCredits(SDL_FRect rect, App* app) {
    return renderTextCenteredToFit(app->state.sdlState.renderer, app->state.sdlState.font, CREDIT_TEXT, false, CREDIT_COLOR, rect, NULL);
}
