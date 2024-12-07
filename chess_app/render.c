#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "Render.h"
#include "EventHandler.h"
#include "Events.h"
#include "AppStyle.h"

#include "../src/MoveGenerator.h"
#include "../src/state/Piece.h"

#include <stdio.h>
#include <stdint.h>

static void formatTime(u32 milliseconds, char *output, size_t outputSize) {
    if (!output || outputSize < 6) {
        return;
    }

    u32 totalSeconds = milliseconds / 1000;
    u32 minutes = totalSeconds / 60;
    u32 seconds = totalSeconds % 60;
    // Format the string as "mm:ss"
    snprintf(output, outputSize, "%02u:%02u", minutes, seconds);
}

/**
 * @brief Enum created because I hate bool params
 */
typedef enum Side {
    TOP,
    BOTTOM
} Side;

static void renderTimeControl(SDL_Renderer *renderer, TTF_Font *font, char* timeText, Side side) {
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, timeText, WHITE_COLOR);
    if (textSurface == NULL) { printf("Text Surface is NULL\n"); return; }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) { printf("Text Texture is NULL\n"); SDL_FreeSurface(textSurface); return; }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;

    SDL_Rect timeControlRect;
    timeControlRect.w = textWidth + BUTTON_PADDING;
    timeControlRect.h = textHeight + BUTTON_PADDING;
    timeControlRect.x = PLACEHOLDER_X + (PLACEHOLDER_WIDTH - textWidth) / 2;
    if (side == BOTTOM) {
        timeControlRect.y = PLACEHOLDER_Y + PLACEHOLDER_HEIGHT - timeControlRect.h - BUTTON_PADDING;
    } else {
        timeControlRect.y = PLACEHOLDER_Y + BUTTON_PADDING;
    }

    SDL_Rect textRect = {
        .x = timeControlRect.x + (timeControlRect.w - textWidth) / 2,
        .y = timeControlRect.y + (timeControlRect.h - textHeight) / 2,
        .w = textWidth, 
        .h = textHeight
    };

    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &timeControlRect);

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

#define TIME_TEXT_LENGTH (6)

static void renderTimeControls(SDL_Renderer *renderer, TTF_Font *font, GameState *gameState) {
    char blackTimeText[TIME_TEXT_LENGTH];
    formatTime(gameState->currentState.blackTimeMs, blackTimeText, TIME_TEXT_LENGTH);
    char whiteTimeText[TIME_TEXT_LENGTH]; 
    formatTime(gameState->currentState.whiteTimeMs, whiteTimeText, TIME_TEXT_LENGTH);

    renderTimeControl(renderer, font, gameState->playerColor == WHITE ? blackTimeText : whiteTimeText, TOP);
    renderTimeControl(renderer, font, gameState->playerColor == WHITE ? whiteTimeText : blackTimeText, BOTTOM);
}

static SDL_Rect renderGameStateText(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState) {
    // Right now, this value needs to be checked everytime we add a new 
    // game result so that we don't have a buffer overflow. So yeah, this
    // is not very optimal
    char text[31];

    switch (gameState->result) {
    case GAME_IS_NOT_DONE:
        memcpy(text, "Game is on!", 12);
        break;
    case THREE_MOVE_REPETITION:
        memcpy(text, "Draw by\n repetition", 20);
        break;
    case STALEMATE:
        memcpy(text, "Stalemate", 10);
        break;
    case INSUFFICIENT_MATERIAL:
        memcpy(text, "Draw by\ninsufficient material", 30);
        break;
    case FIFTY_MOVE_RULE:
        memcpy(text, "Draw by\nfifty move rule", 24);
        break;
    case WHITE_WON_CHECKMATE:
        memcpy(text, "White won\nby checkmate", 23);
        break; 
    case BLACK_WON_CHECKMATE:
        memcpy(text, "Black won\nby checkmate", 23);
        break;   
    case WHITE_WON_ON_TIME:
        memcpy(text, "White won\non time", 18);
        break;  
    case BLACK_WON_ON_TIME:
        memcpy(text, "Black won\non time", 18);
        break;      
    default:
        memcpy(text, "Error on switch", 16);
        break;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text, WHITE_COLOR, 0);
    if (textSurface == NULL) { printf("Text Surface is NULL\n"); return (SDL_Rect) { -1, -1, -1, -1 }; }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) { printf("Text Texture is NULL\n");  SDL_FreeSurface(textSurface); return (SDL_Rect) { -1, -1, -1, -1 }; }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect textRect = {
        .x = PLACEHOLDER_X + (PLACEHOLDER_WIDTH - textWidth) / 2, // Center horizontally
        .y = PLACEHOLDER_Y + (PLACEHOLDER_HEIGHT - textHeight) / 2, // Center vertically
        .w = textWidth,
        .h = textHeight
    };
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);

    return textRect;
}

static int restartButtonIndex = -1;

static SDL_Rect renderRestartButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect textRect, ClickableAreas *clickableAreas) {
    // Render the "Restart" button text
    SDL_Surface *buttonTextSurface = TTF_RenderText_Solid(font, "Restart", BUTTON_TEXT_COLOR);
    if (buttonTextSurface == NULL) { printf("Button Text Surface is NULL\n"); return (SDL_Rect) { -1, -1, -1, -1 }; }
    SDL_Texture *buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Button Text Texture is NULL\n"); SDL_FreeSurface(buttonTextSurface); return (SDL_Rect) { -1, -1, -1, -1 }; }
    
    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = { 
        .x = PLACEHOLDER_X + BUTTON_PADDING, 
        .y = textRect.y + textRect.h + BUTTON_PADDING, 
        .w = buttonWidth, 
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth, 
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);

    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedRestartButton
    };
    if (restartButtonIndex == -1) {
        restartButtonIndex = clickableAreas->count;
        da_append(clickableAreas, area);
    } else {
        da_update(clickableAreas, restartButtonIndex, area);
    }

    return buttonRect;
}

static int switchColorButtonIndex = -1;

static void renderSwitchColorButton(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect restartButtonRect, ClickableAreas *clickableAreas) {
    // Render the "Restart" button text
    SDL_Surface *buttonTextSurface = TTF_RenderText_Blended_Wrapped(font, "Switch\nColor", BUTTON_TEXT_COLOR, 0);
    if (buttonTextSurface == NULL) { printf("Button Text Surface is NULL\n"); return; }
    SDL_Texture *buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Button Text Texture is NULL\n"); SDL_FreeSurface(buttonTextSurface); return; }
    
    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;

    int buttonWidth = buttonTextWidth + BUTTON_PADDING;
    int buttonHeight = buttonTextHeight + BUTTON_PADDING;
    SDL_Rect buttonRect = { 
        .x = PLACEHOLDER_X + PLACEHOLDER_WIDTH - buttonWidth - BUTTON_PADDING, 
        .y = restartButtonRect.y, 
        .w = buttonWidth, 
        .h = buttonHeight };

    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth, 
        .h = buttonTextHeight
    };

    // Render the button background
    SDL_SetRenderDrawColor(renderer, BUTTON_COLOR.r, BUTTON_COLOR.g, BUTTON_COLOR.b, BUTTON_COLOR.a);
    SDL_RenderFillRect(renderer, &buttonRect);
    
    
    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedSwitchColorButton
    };
    if (switchColorButtonIndex == -1) {
        switchColorButtonIndex = clickableAreas->count;
        da_append(clickableAreas, area);
    } else {
        da_update(clickableAreas, switchColorButtonIndex, area);
    }
}

static void renderPlaceholder(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState, ClickableAreas *clickableAreas) {
    SDL_Rect placeholderRect = PLACEHOLDER_RECT;
    // Draw rectangle border
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR.r, BACKGROUND_COLOR.g, BACKGROUND_COLOR.b, BACKGROUND_COLOR.a);
    SDL_RenderDrawRect(renderer, &placeholderRect);

    SDL_Rect textRect = renderGameStateText(renderer, font, gameState);
    if (textRect.x == -1) {
        printf("ERROR while rendering the game state text\n");
        return;
    }
    SDL_Rect buttonRect = renderRestartButton(renderer, font, textRect, clickableAreas);
    if (buttonRect.x == -1) {
        printf("ERROR while rendering the restart button\n");
        return;
    }
    renderSwitchColorButton(renderer, font, buttonRect, clickableAreas);

    renderTimeControls(renderer, font, gameState);
}

static void renderDraggedPiece(SDL_Renderer *renderer,
                               Textures chessImages,
                               DraggingState draggingState,
                               int mouseX, int mouseY) {
    Piece draggedPiece = draggingState.draggedPiece;
    // This should be always false because we are already checking if isDragging is true
    if (draggedPiece == NOPIECE) { return; }

    int indexOffset = pieceColor(draggedPiece) == WHITE ? 9 : 11;
    TextureState chessImageData = chessImages.data[draggedPiece - indexOffset];
    SDL_Rect destRect = {mouseX - chessImageData.width / 2, mouseY - chessImageData.height / 2,
                         chessImageData.width, chessImageData.height};
    SDL_RenderCopy(renderer, chessImageData.texture, NULL, &destRect);
}

static void renderChessboard(SDL_Renderer *renderer,
                             Textures chessImages,
                             GameState *gameState,
                             DraggingState draggingState) {
    SDL_Rect rect = CHESSBOARD_RECT;
    int squareSize = rect.w / BOARD_LENGTH;

    bool flip = gameState->playerColor == BLACK;

    for (int squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);
         if (flip) {
            row = BOARD_LENGTH - 1 - row; // Flip the row index
            col = BOARD_LENGTH - 1 - col; // Flip the column index
        }

        SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect square = {rect.x + col * squareSize, rect.y + row * squareSize, squareSize, squareSize};
        SDL_RenderFillRect(renderer, &square);

        // Don't render the dragged pieces at their position and at the mouse coordinates
        if (draggingState.isDragging && squareIndex == draggingState.from) { continue; } 

        Piece piece = pieceAtIndex(gameState->currentState.currentPosition.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = {square.x, square.y, squareSize, squareSize};
            int index = piece - (pieceColor(piece) == WHITE ? 9 : 11);
            SDL_RenderCopy(renderer, chessImages.data[index].texture, NULL, &pieceRect);
        }
    }
}

void render(AppEvents *appEvents, AppState *appState) {

    SDL_RenderClear(appState->sdlState.renderer);

    renderPlaceholder(appState->sdlState.renderer, appState->sdlState.font, &appState->gameState, &appEvents->clickableAreas);
    renderChessboard(appState->sdlState.renderer, appState->textures, &appState->gameState, appState->draggingState);

    if (appState->draggingState.isDragging) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        renderDraggedPiece(appState->sdlState.renderer, appState->textures, appState->draggingState, mouseX, mouseY);
    }

    SDL_RenderPresent(appState->sdlState.renderer);
}