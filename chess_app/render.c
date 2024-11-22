#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "../src/MoveGenerator.h"
#include "Render.h"
#include "EventHandler.h"

#define SQUARE_COLOR_1 ((SDL_Color) {240, 217, 181, 255})
#define SQUARE_COLOR_2 ((SDL_Color) {181, 136, 99, 255})

static int restartButtonIndex = -1;

static SDL_Rect renderGameStateText(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState) {
    SDL_Color textColor = {0, 0, 0, 255};
    char text[24];

    switch (gameState->result) {
    case GAME_IS_NOT_DONE:
        memcpy(text, "Game is on!", 12);
        break;
    case THREE_MOVE_REPETITION:
        memcpy(text, "Draw by repetition", 19);
        break;
    case STALEMATE:
        memcpy(text, "Stalemate", 10);
        break;
    case FIFTY_MOVE_RULE:
        memcpy(text, "Draw by fifty move rule", 24);
        break;
    case WHITE_WON_CHECKMATE:
        memcpy(text, "White won by checkmate", 23);
        break; 
    case BLACK_WON_CHECKMATE:
        memcpy(text, "Black won by checkmate", 23);
        break;   
    case WHITE_WON_ON_TIME:
        memcpy(text, "White won on time", 18);
        break;  
    case BLACK_WON_ON_TIME:
        memcpy(text, "Black won on time", 18);
        break;      
    default:
        memcpy(text, "Error on switch", 16);
        break;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (textSurface == NULL) { printf("Text Surface is NULL\n"); return; }
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) { printf("Text Texture is NULL\n");  SDL_FreeSurface(textSurface); return; }

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

static void renderRestartButton(SDL_Renderer* renderer, TTF_Font* font, GameState* state, SDL_Rect textRect, ClickableAreas *clickableAreas) {
    int padding = 20;
    int buttonWidth = PLACEHOLDER_WIDTH / 3;
    int buttonHeight = PLACEHOLDER_HEIGHT / 8;
    SDL_Rect buttonRect = { 
        .x = PLACEHOLDER_X + (PLACEHOLDER_WIDTH - buttonWidth) / 2, 
        .y = textRect.y + textRect.h + padding, 
        .w = buttonWidth, 
        .h = buttonHeight };

    SDL_Color buttonColor = {100, 149, 237, 255}; // Cornflower blue for the button
    SDL_Color buttonTextColor = {255, 255, 255, 255}; // White color for button text

    // Render the button background
    SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer, &buttonRect);

    // Render the "Restart" button text
    SDL_Surface *buttonTextSurface = TTF_RenderText_Solid(font, "Restart", buttonTextColor);
    if (buttonTextSurface == NULL) { printf("Button Text Surface is NULL\n"); return; }
    SDL_Texture *buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
    if (buttonTextSurface == NULL) { printf("Button Text Texture is NULL\n"); SDL_FreeSurface(buttonTextSurface); return; }
    
    int buttonTextWidth = buttonTextSurface->w;
    int buttonTextHeight = buttonTextSurface->h;
    SDL_Rect buttonTextRect = {
        .x = buttonRect.x + (buttonRect.w - buttonTextWidth) / 2,
        .y = buttonRect.y + (buttonRect.h - buttonTextHeight) / 2,
        .w = buttonTextWidth, 
        .h = buttonTextHeight
    };
    SDL_FreeSurface(buttonTextSurface);
    SDL_RenderCopy(renderer, buttonTextTexture, NULL, &buttonTextRect);
    SDL_DestroyTexture(buttonTextTexture);

    ClickableArea area = {
        .rect = buttonRect,
        .callback = &clickedRestartButton
    };
    if (restartButtonIndex == -1) {
        restartButtonIndex = clickableAreas->count;
        clickableAreas_append(clickableAreas, area);
    } else {
        clickableAreas_update(clickableAreas, restartButtonIndex, area);
    }
}

static void renderPlaceholder(SDL_Renderer* renderer, TTF_Font* font, GameState* gameState, ClickableAreas *clickableAreas) {
    SDL_Rect placeholderRect = PLACEHOLDER_RECT;
    // Draw rectangle border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &placeholderRect);

    SDL_Rect textRect = renderGameStateText(renderer, font, gameState);
    renderRestartButton(renderer, font, gameState, textRect, clickableAreas);
}

static void renderDraggedPiece(SDL_Renderer *renderer,
                               ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE],
                               DraggingState draggingState,
                               int mouseX, int mouseY) {
    Piece draggedPiece = draggingState.draggedPiece;
    if (draggedPiece == NOPIECE) { return; }
    ImageData chessImageData = chessImages[pieceColor(draggedPiece) / WHITE - 1][pieceType(draggedPiece) - 1];
    SDL_Rect destRect = {mouseX - chessImageData.width / 2, mouseY - chessImageData.height / 2,
                         chessImageData.width, chessImageData.height};
    SDL_RenderCopy(renderer, chessImageData.texture, NULL, &destRect);
}

static void renderChessboard(SDL_Renderer* renderer, 
                              ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], 
                              GameState* gameState,
                              DraggingState draggingState) {
    SDL_Rect rect = CHESSBOARD_RECT;
    int squareSize = rect.w / BOARD_LENGTH;

    for (int squareIndex = 0; squareIndex < BOARD_SIZE; squareIndex++) {
        int row = rank(squareIndex);
        int col = file(squareIndex);
        SDL_Color color = ((row + col) % 2 == 0) ? SQUARE_COLOR_1 : SQUARE_COLOR_2;
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect square = {rect.x + col * squareSize, rect.y + row * squareSize, squareSize, squareSize};
        SDL_RenderFillRect(renderer, &square);

        // Don't render the dragged pieces at their position and at the mouse coordinates
        if (draggingState.isDragging && squareIndex == draggingState.from) { continue; } 

        Piece piece = pieceAtIndex(gameState->currentState.currentPosition.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = {square.x, square.y, squareSize, squareSize};
            SDL_RenderCopy(renderer, chessImages[pieceColor(piece) / WHITE - 1][pieceType(piece) - 1].texture, NULL, &pieceRect);
        }
    }
}

void render(SDL_State* sdlState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], GameState* gameState, DraggingState draggingState, ClickableAreas *clickableAreas) {

    SDL_RenderClear(sdlState->renderer);

    renderPlaceholder(sdlState->renderer, sdlState->font, gameState, clickableAreas);
    renderChessboard(sdlState->renderer, chessImages, gameState, draggingState);

    if (draggingState.isDragging) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        renderDraggedPiece(sdlState->renderer, chessImages, draggingState, mouseX, mouseY);
    }

    SDL_RenderPresent(sdlState->renderer);
}