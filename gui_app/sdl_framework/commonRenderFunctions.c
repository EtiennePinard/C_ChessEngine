#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "CommonRenderFunctions.h"

SDL_AppResult findFontSizeToFit(const char* textString, SDL_FRect rectToFit, TTF_Font* baseFont, bool isTextMultiLine, float* resultingFontSize) {
    SDL_AppResult result = SDL_APP_FAILURE;
    if (!baseFont || !textString) return result;

    TTF_Font* tempFont = TTF_CopyFont(baseFont);
    if (!tempFont) return result;

    // If we have an empty string simply return the copied font
    if (textString[0] == '\0') {
        *resultingFontSize = 0.0;
        result = SDL_APP_CONTINUE;
        goto end;
    }

    // Initial guess font size
    const float referenceSize = 16.0;

    // Set base font to reference size
    if (!TTF_SetFontSize(tempFont, referenceSize)) goto end;

    int textW = 0, textH = 0;
    bool sizeSuccess = false;
    if (isTextMultiLine) sizeSuccess = TTF_GetStringSizeWrapped(tempFont, textString, 0, 0, &textW, &textH);
    else sizeSuccess = TTF_GetStringSize(tempFont, textString, 0, &textW, &textH);

    if (!sizeSuccess || textW == 0 || textH == 0) goto end;

    // Compute scale factor based on rectToFit size
    float scaleW = (float)rectToFit.w / (float)textW;
    float scaleH = (float)rectToFit.h / (float)textH;
    float scale = fminf(scaleW, scaleH);

    float finalSize = referenceSize * scale;
    finalSize = SDL_clamp(finalSize, 4, rectToFit.h); // Prevent tiny or huge font sizes

    *resultingFontSize = finalSize;
    result = SDL_APP_CONTINUE;
end:
    TTF_CloseFont(tempFont);
    return result;
}

SDL_AppResult renderMultilineTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rect) {
    if (!renderer || !font || !text) return SDL_APP_FAILURE;

    TTF_Font* tempFont = TTF_CopyFont(font);
    if (!tempFont) return SDL_APP_FAILURE;

    float fontSizeToFit;
    if (findFontSizeToFit(text, rect, tempFont, true, &fontSizeToFit) != SDL_APP_CONTINUE) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }
    if (!TTF_SetFontSize(tempFont, fontSizeToFit)) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    char* textCopy = strdup(text);
    if (!textCopy) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    int lineHeight = TTF_GetFontLineSkip(tempFont);
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
        SDL_Surface* surf = TTF_RenderText_Blended(tempFont, line, 0, color);
        if (!surf) {
            TTF_CloseFont(tempFont);
            free(textCopy);
            return SDL_APP_FAILURE;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        if (!tex) {
            TTF_CloseFont(tempFont);
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

    TTF_CloseFont(tempFont);
    free(textCopy);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderSingleLineTextCenteredToFit(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color color, SDL_FRect rect) {
    if (!renderer || !font || !text) return SDL_APP_FAILURE;

    TTF_Font* tempFont = TTF_CopyFont(font);
    if (!tempFont) return SDL_APP_FAILURE;

    float fontSizeToFit;
    if (findFontSizeToFit(text, rect, tempFont, false, &fontSizeToFit) != SDL_APP_CONTINUE) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }
    if (!TTF_SetFontSize(tempFont, fontSizeToFit)) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(tempFont, text, 0, color, 0);
    if (!textSurface) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) { SDL_DestroySurface(textSurface); TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    // Centering the text from the baseline
    int ascent = TTF_GetFontAscent(tempFont);
    SDL_FRect renderTextRect = {
    .x = rect.x + (rect.w - textSurface->w) / 2,
    .y = rect.y + (rect.h - ascent) / 2,
    .w = textSurface->w,
    .h = textSurface->h
    };
    SDL_RenderTexture(renderer, textTexture, NULL, &renderTextRect);
    TTF_CloseFont(tempFont);
    SDL_DestroySurface(textSurface);
    SDL_DestroyTexture(textTexture);
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderButton(SDL_FRect rect, App* app, int hoverIndex, const char* text,
    SDL_Color hoverColor, SDL_Color clickedColor, SDL_Color idleColor, SDL_Color borderColor, SDL_Color textColor) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

    // Background
    if (app->events.mouseState.hoveredIndex == hoverIndex &&
        SDL_PointInRectFloat(&app->events.mouseState.mousePoint, &rect)) {
        if (app->events.mouseState.holdingLeftMouseButton) {
            // Rect is being currently clicked, use clickedColor 
            SDL_SetRenderDrawColor(renderer, clickedColor.r, clickedColor.g, clickedColor.b, clickedColor.a);
        }
        else {
            // This is the currently hovered box, use highlight color
            SDL_SetRenderDrawColor(renderer, hoverColor.r, hoverColor.g, hoverColor.b, hoverColor.a);
        }
    }
    else {
        // Use idle color if not hoverered by mouse
        SDL_SetRenderDrawColor(renderer, idleColor.r, idleColor.g, idleColor.b, idleColor.a);
    }
    SDL_RenderFillRect(renderer, &rect);

    // Border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_RenderRect(renderer, &rect);

    // Render text with only width padding
    const float textWidthPaddingPercent = 0.1f;
    const float textWidthPadding = textWidthPaddingPercent * rect.w;
    SDL_FRect paddedRect = {
        rect.x + textWidthPadding / 2,
        rect.y,
        rect.w - textWidthPadding,
        rect.h
    };
    return renderSingleLineTextCenteredToFit(renderer, font, text, textColor, paddedRect);
}

SDL_AppResult renderLabeledCheckboxButton(SDL_FRect rect, App* app, bool checked, const char* label, int hoveredIndex,
    SDL_Color borderColor, SDL_Color highlightColor, SDL_Color checkedColor, SDL_Color textColor) {
    SDL_Renderer* renderer = app->state.sdlState.renderer;
    TTF_Font* font = app->state.sdlState.font;

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
        SDL_SetRenderDrawColor(renderer, checkedColor.r, checkedColor.g, checkedColor.b, checkedColor.a);
        SDL_RenderFillRect(renderer, &checkboxRect);
    }

    // Draw label to the right of the box
    SDL_FRect textRect = {
        .x = checkboxRect.x + checkboxRect.w + rect.h * 0.3f,
        .y = rect.y,
        .w = rect.w - checkboxRect.w - rect.h * 0.3f,
        .h = rect.h
    };

    return renderSingleLineTextCenteredToFit(renderer, font, label, textColor, textRect);
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

SDL_AppResult drawCaret(App* app, SDL_Color color, SDL_FRect rect, float caretX) {
    // Draw the blinking caret
    if (app->events.textInput.cursor.showCursor) {
        const float caretPadding = 2.0f;
        SDL_SetRenderDrawColor(app->state.sdlState.renderer, color.r, color.g, color.b, color.a);

        SDL_FRect caret = {
            .x = caretX,
            .y = rect.y + caretPadding,
            .w = 1.5f,
            .h = rect.h - 2.0f * caretPadding
        };

        SDL_RenderFillRect(app->state.sdlState.renderer, &caret);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult renderTextInputCenteredToFit(SDL_FRect rect, App* app, SDL_Color highlightColor,
    SDL_Color unselectedTextColor, SDL_Color selectedTextColor, SDL_Color selectedTextBgColor) {
    if (!app->events.textInput.isActive) return SDL_APP_CONTINUE;

    if (app->events.textInput.text.count == 0) {
        // There is no characters to draw, only draw the cursor
        float caretX = rect.x + rect.w / 2.0;
        SDL_Rect area = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
        SDL_SetTextInputArea(app->state.sdlState.window, &area, (int)(caretX - rect.x));
        return drawCaret(app, unselectedTextColor, rect, caretX);
    }

    TTF_Font* font = app->state.sdlState.font;
    TTF_Font* tempFont = TTF_CopyFont(font);
    if (!tempFont) return SDL_APP_FAILURE;

    const char* text = app->events.textInput.text.data;
    float fontSizeToFit;
    if (findFontSizeToFit(text, rect, tempFont, false, &fontSizeToFit) != SDL_APP_CONTINUE) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }
    if (!TTF_SetFontSize(tempFont, fontSizeToFit)) { TTF_CloseFont(tempFont); return SDL_APP_FAILURE; }

    SDL_Renderer* renderer = app->state.sdlState.renderer;

    // Highlight the border
    SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
    SDL_RenderRect(renderer, &rect);

    float totalWidth = 0.0f;

    size_t textLen = app->events.textInput.text.count;
    size_t cursorIndex = app->events.textInput.cursor.index;

    int advance;
    // First pass: measure total width
    for (size_t i = 0; i < textLen; i++) {
        if (!TTF_GetGlyphMetrics(tempFont, (Uint8)text[i], NULL, NULL, NULL, NULL, &advance)) return SDL_APP_FAILURE;
        totalWidth += (float)advance;
    }

    // Starting point for centered rendering
    float startX = rect.x + (rect.w - totalWidth) / 2.0f;
    float baselineY = rect.y + (rect.h - TTF_GetFontAscent(tempFont)) / 2.0f;

    float penX = startX;
    float caretX = startX;

    // Resetting the glyph rects
    app->events.textInput.glyphRects.count = 0;

    // Normalize selection range
    size_t selectionStart = SDL_min(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);
    size_t selectionEnd = SDL_max(app->events.textInput.selectionStart, app->events.textInput.selectionStart + app->events.textInput.nbCharSelected);

    SDL_Color textColor = unselectedTextColor;

    for (size_t i = 0; i < textLen; i++) {
        Uint8 ch = (Uint8)text[i];

        if (!TTF_GetGlyphMetrics(tempFont, ch, NULL, NULL, NULL, NULL, &advance)) return SDL_APP_FAILURE;

        // Set the correct caret position
        if (i == cursorIndex) caretX = penX;

        if (selectionStart <= i && i < selectionEnd) textColor = selectedTextColor;
        else textColor = unselectedTextColor;

        SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(tempFont, ch, textColor);
        if (!glyphSurface) return SDL_APP_FAILURE;

        SDL_Texture* glyphTexture = SDL_CreateTextureFromSurface(renderer, glyphSurface);
        if (!glyphTexture) { SDL_DestroySurface(glyphSurface); return SDL_APP_FAILURE; }

        SDL_FRect dst = {
            .x = penX,
            .y = baselineY,
            .w = (float)glyphSurface->w,
            .h = (float)glyphSurface->h
        };
        da_append((&app->events.textInput.glyphRects), dst);

        // Render the selection color behind the text
        if (selectionStart <= i && i < selectionEnd) {
            SDL_SetRenderDrawColor(renderer, selectedTextBgColor.r, selectedTextBgColor.g, selectedTextBgColor.b, selectedTextBgColor.a);
            const float selectionPadding = 2.0f;
            SDL_FRect selectionRect = { penX, rect.y + selectionPadding, (float)glyphSurface->w, rect.h - 2.0f * selectionPadding };
            SDL_RenderFillRect(renderer, &selectionRect);
        }

        SDL_RenderTexture(renderer, glyphTexture, NULL, &dst);
        SDL_DestroyTexture(glyphTexture);

        penX += (float)advance;
    }

    // If cursor is at end of string
    if (cursorIndex == textLen) caretX = penX;

    SDL_Rect area = { (int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h };
    SDL_SetTextInputArea(app->state.sdlState.window, &area, (int)(caretX - rect.x));
    return drawCaret(app, unselectedTextColor, rect, caretX);
}
