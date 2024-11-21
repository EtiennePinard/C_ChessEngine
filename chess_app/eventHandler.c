#include "EventHandler.h"

#include "../src/state/Board.h"
#include "../src/state/Move.h"
#include "../src/MoveGenerator.h"
#include "../src/ChessGameEmulator.h"
#include "../src/chessBot/ChessBot.h"

static bool pointInChessBoard(int x, int y) {
    return x >= CHESSBOARD_X && x < CHESSBOARD_X + CHESSBOARD_WIDTH &&
        y >= CHESSBOARD_Y && y < CHESSBOARD_Y + CHESSBOARD_HEIGHT;
}

// Note: This will be correct if the point (x, y) is in the chessboard
static int squareFromxy(int x, int y) {
    int squareSize = CHESSBOARD_WIDTH / BOARD_LENGTH;
    int col = (x - CHESSBOARD_X) / squareSize;
    int row = y / squareSize;
    return row * BOARD_LENGTH + col;
}

void handleMouseButtonDown(GameState *gameState, DraggingState *draggingState) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    if (!pointInChessBoard(mouseX, mouseY)) { return; }
    
    int square = squareFromxy(mouseX, mouseY);

    if (pieceAtIndex(gameState->currentState.currentPosition.board, square) == NOPIECE) { return; }

    draggingState->draggedPiece = pieceAtIndex(gameState->currentState.currentPosition.board, square);
    draggingState->from = square;
    draggingState->isDragging = true;
}

static void playBotMove(GameState *gameState) {
    provideGameStateForBot(&gameState->currentState);
    Move botMove = think();
    if (gameState->previousStateIndex >= gameState->previousStateCapacity) {
        gameState->previousStates = realloc(gameState->previousStates, sizeof(GameState) * gameState->previousStateCapacity * 2);
        gameState->previousStateCapacity *= 2;
    }
    gameState->previousStates[gameState->previousStateIndex++] = gameState->currentState;

    playMove(botMove, &gameState->currentState);
}

void handleMouseButtonUp(GameState *gameState, DraggingState *draggingState) {
    draggingState->isDragging = false; // Always stop dragging when we stop holding click

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    if (!pointInChessBoard(mouseX, mouseY)) { return; }
    
    int draggingTo = squareFromxy(mouseX, mouseY);

    // Finding the valid moves of this position
    // We could cache this value if it really is that slow, but I don't think so
    // Here I used 256 because it is the closest power of 2 from MAX_LEGAL_MOVES
    Move moves[256];
    int numMoves;
    getValidMoves(moves, &numMoves, gameState->currentState.currentPosition);

    for (int moveIndex = 0; moveIndex < numMoves; moveIndex++) {
        Move move = moves[moveIndex];

        if (fromSquare(move) == draggingState->from && toSquare(move) == draggingTo) {

            if (gameState->previousStateIndex >= gameState->previousStateCapacity) {
                gameState->previousStates = realloc(gameState->previousStates, sizeof(GameState) * gameState->previousStateCapacity * 2);
                gameState->previousStateCapacity *= 2;
            }
            gameState->previousStates[gameState->previousStateIndex++] = gameState->currentState;
            // This is the move the player wants to play
            // This is true because their is only one move with a particular from and to square
            playMove(move, &gameState->currentState);

            playBotMove(gameState);
            break;
        }
    }
}
