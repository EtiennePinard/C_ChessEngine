#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#include "../chess_app/AppInit.h"
#include "../chess_app/AppState.h"
#include "../chess_app/AppStyle.h"

#include "../src/utils/Math.h"
#include "../src/bot/PieceSquareTable.h"
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/state/ZobristKey.h"

typedef struct {
    SDL_Rect rect;
    SDL_Color color;
    const char *text;
} Button;

typedef struct TextureState {
    SDL_Texture *texture;
    int width;
    int height;
} TextureState;

typedef struct Textures {
    TextureState *data;
    size_t count;
    size_t capacity;
} Textures;


static void renderChessboard(
    SDL_Renderer *renderer,
    Textures chessImages,
    GameState *gameState,
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

        Piece piece = Board_pieceAtIndex(gameState->currentState.board, squareIndex);
        if (piece != NOPIECE) {
            SDL_Rect pieceRect = {square.x, square.y, squareSize, squareSize};
            int index = piece - (Piece_color(piece) == WHITE ? 9 : 11);
            SDL_RenderCopy(renderer, chessImages.data[index].texture, NULL, &pieceRect);
        }
    }
}


int main() {
    // Reusing the same code from app.c
    AppState appState = { 0 };
    AppEvents appEvents = { 0 };

    if (!initializeApp(&appEvents, &appState)) {
        fprintf(stderr, "Failed to initialize application.\n");
        exit(EXIT_FAILURE);
    }
    
    while (appState.isRunning) {
        handleEvent(&appEvents, &appState);
        render(&appEvents, &appState);
    }

    cleanupApp(&appEvents, &appState);

    return 0;
}
