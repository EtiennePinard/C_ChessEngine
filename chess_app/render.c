#include <stdbool.h>
#include <SDL2/SDL_image.h>

#include "../src/MoveGenerator.h"
#include "Render.h"

#define SQUARE_COLOR_1 ((SDL_Color) {240, 217, 181, 255})
#define SQUARE_COLOR_2 ((SDL_Color) {181, 136, 99, 255})

static void renderPlaceholder(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect rect, GameState* gameState) {
    // Draw rectangle border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    // Render text inside the rectangle
    SDL_Color textColor = {0, 0, 0, 255};
    char text[12];

    Move moves[256];
    int numMove;
    getValidMoves(moves, &numMove, gameState->currentState.currentPosition);
    if (isKingInCheck() && numMove == 0) {
        memcpy(text, "Checkmate", 10);
    } else {
        memcpy(text, "Game is on!", 12);
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (textSurface) {
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            SDL_Rect textRect;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
            textRect.x = rect.x + (rect.w - textRect.w) / 2; // Center horizontally
            textRect.y = rect.y + (rect.h - textRect.h) / 2; // Center vertically
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
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
                              SDL_Rect rect, 
                              ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], 
                              GameState* gameState,
                              DraggingState draggingState) {
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

void render(SDL_State* sdlState, ImageData chessImages[NB_PIECE_COLOR][NB_PIECE_TYPE], GameState* gameState, DraggingState draggingState) {
    SDL_Rect placeholderRect = { .x = PLACEHOLDER_X, .y = PLACEHOLDER_Y, .w = PLACEHOLDER_WIDTH, .h = PLACEHOLDER_HEIGHT };
    SDL_Rect chessboardRect = { .x = CHESSBOARD_X, .y = CHESSBOARD_Y, .w = CHESSBOARD_WIDTH, .h = CHESSBOARD_HEIGHT };

    // SDL_SetRenderDrawColor(sdlState->renderer, 255, 255, 255, 255);
    SDL_RenderClear(sdlState->renderer);


    renderPlaceholder(sdlState->renderer, sdlState->font, placeholderRect, gameState);
    renderChessboard(sdlState->renderer, chessboardRect, chessImages, gameState, draggingState);

    if (draggingState.isDragging) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        renderDraggedPiece(sdlState->renderer, chessImages, draggingState, mouseX, mouseY);
    }

    SDL_RenderPresent(sdlState->renderer);
}