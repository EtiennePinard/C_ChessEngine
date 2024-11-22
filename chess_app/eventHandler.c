#include "EventHandler.h"

#include "../src/state/Board.h"
#include "../src/state/Move.h"
#include "../src/MoveGenerator.h"
#include "../src/ChessGameEmulator.h"
#include "../src/chessBot/ChessBot.h"

static void resetGame(GameState *gameState) {
    if (gameState->previousStateIndex == 0) {
        return; // Game is already reset
    }
    gameState->currentState = gameState->previousStates[0];
    gameState->previousStateIndex = 0;
    gameState->result = GAME_IS_NOT_DONE;
}

void clickedRestartButton(SDL_Event event, GameState *gameState, DraggingState *draggingState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        resetGame(gameState);
        break;
    default: // Only do something for mouse button down
        break;
    }
}

// Note: This will be correct if the point (x, y) is in the chessboard
static int squareFromxy(int x, int y) {
    int squareSize = CHESSBOARD_WIDTH / BOARD_LENGTH;
    int col = (x - CHESSBOARD_X) / squareSize;
    int row = y / squareSize;
    return row * BOARD_LENGTH + col;
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

static void computeGameEnd(GameState *gameState) {
    Move moves[256];
    int numMove;
    getValidMoves(moves, &numMove, gameState->currentState.currentPosition);
    if (numMove == 0) {
        if (isKingInCheck() || isKingInDoubleCheck()) {
            gameState->result = gameState->currentState.currentPosition.colorToGo == WHITE ? BLACK_WON_CHECKMATE : WHITE_WON_CHECKMATE;
        } else {
            gameState->result = STALEMATE;
        }
    } else if (isThereThreeFoldRepetition(&gameState->currentState)) {
        gameState->result = THREE_MOVE_REPETITION;
    } else if (gameState->currentState.currentPosition.turnsForFiftyRule > 50) {
        // TODO: Check this case out
        gameState->result = FIFTY_MOVE_RULE;
    }
}

static void chessBoardMouseButtonUp(GameState *gameState, DraggingState *draggingState) {
    if (!draggingState->isDragging) { return; } // We are not dragging anything
    draggingState->isDragging = false; // Always stop dragging when we stop holding click

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
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
            // This is true because their is only one move with a particular from and to square (except for promotion)
            playMove(move, &gameState->currentState);
            computeGameEnd(gameState);
            if (gameState->result != GAME_IS_NOT_DONE) { break; }

            playBotMove(gameState);
            break;
        }
    }
}

static void chessBoardMouseButtonDown(GameState *gameState, DraggingState *draggingState) {
    if (gameState->result != GAME_IS_NOT_DONE) { return; } // Game is done

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    int square = squareFromxy(mouseX, mouseY);

    if (pieceAtIndex(gameState->currentState.currentPosition.board, square) == NOPIECE) { return; }

    draggingState->draggedPiece = pieceAtIndex(gameState->currentState.currentPosition.board, square);
    draggingState->from = square;
    draggingState->isDragging = true;
}

void clickeChessBoard(SDL_Event event, GameState *gameState, DraggingState *draggingState) {
    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN:
        chessBoardMouseButtonDown(gameState, draggingState);
        break;
    case SDL_MOUSEBUTTONUP:
        chessBoardMouseButtonUp(gameState, draggingState);
        break;
    default:
        break;
    }
}

static bool pointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h;
}

void handleEvent(bool *isRunning, ClickableAreas *clickableAreas, GameState *gameState, DraggingState *draggingState) {
    SDL_Event event;
        int mouseX, mouseY;
            while (SDL_PollEvent(&event)) {
            // TODO: Maybe put this logic in the event handler file
            switch (event.type) {
            case SDL_QUIT:
                *isRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                SDL_GetMouseState(&mouseX, &mouseY);
                for (int index = 0; index < clickableAreas->count; index++) {
                    ClickableArea area = clickableAreas->areas[index];
                    if (pointInRect(mouseX, mouseY, area.rect)) {
                        area.callback(event, gameState, draggingState);
                    }
                }
                break;
            default:
                break;
            }
        }
}