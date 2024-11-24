#include "Overlay.h"

void renderPromotionOverlay(SDL_Renderer* renderer, 
                            Textures chessImages,
                            PieceCharacteristics colorToPromote,
                            int promotionSquare,
                            Popup *popup) {
    // Chessboard dimensions
    int squareSize = (WINDOW_WIDTH * 2 / 3) / BOARD_LENGTH;

    // Calculate position of the promotion square
    int promotionFile = file(promotionSquare);
    int promotionRank = rank(promotionSquare);
    int squareX = CHESSBOARD_X + promotionFile * squareSize;
    int squareY = promotionRank * squareSize;

    // Overlay dimensions
    int overlayWidth = 2 * squareSize; // Two options per row
    int overlayHeight = 2 * squareSize; // Two rows of options
    int overlayX = squareX - squareSize; // Centered relative to the square
    int overlayY = (promotionRank == 0) ? squareY : squareY - overlayHeight; // Adjust for promotion direction

    // Render overlay background
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray background
    SDL_Rect overlayRect = {overlayX, overlayY, overlayWidth, overlayHeight};
    SDL_RenderFillRect(renderer, &overlayRect);

    // Draw borders for better visibility
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
    SDL_RenderDrawRect(renderer, &overlayRect);

    // Render piece textures
    int nbTexture = 4;
    int indexOffSet = colorToPromote == WHITE ? 9 : 11;
    SDL_Texture *textures[nbTexture]; 
    textures[0] = chessImages.data[makePiece(colorToPromote, QUEEN) - indexOffSet].texture;
    textures[1] = chessImages.data[makePiece(colorToPromote, KNIGHT) - indexOffSet].texture;
    textures[2] = chessImages.data[makePiece(colorToPromote, ROOK) - indexOffSet].texture;
    textures[3] = chessImages.data[makePiece(colorToPromote, BISHOP) - indexOffSet].texture;
    
    for (int i = 0; i < nbTexture; i++) {
        int pieceCol = i % 2; // Column within the grid 
        int pieceRow = i / 2; // Row within the grid
        SDL_Rect pieceRect = {
            overlayX + pieceCol * squareSize,
            overlayY + pieceRow * squareSize,
            squareSize,
            squareSize
        };
        SDL_RenderCopy(renderer, textures[i], NULL, &pieceRect);
    }
    SDL_RenderPresent(renderer);

    popup->rect = overlayRect;
}
