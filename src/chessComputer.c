#include "chessComputer.h"
#include "chessGameEmulator.h"
#include "../testing/logChessStructs.h"

// max function copied from StackOverflow
#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

int _pieceToValue(int piece) {
    int perspective = (piece & pieceColorBitMask) == WHITE ? 1 : -1;
    switch (piece & pieceTypeBitMask) {
    case PAWN:   return _pawnValue * perspective;
    case KNIGHT: return _knightValue * perspective;
    case BISHOP: return _bishopValue * perspective;
    case ROOK:   return _rookValue * perspective;
    case QUEEN:  return _queenValue * perspective;
    default:     return 0;
    }
}

/**
 * Only checks at material and mobility
 * Does not look into king safety, center control, ect...
 * Returns a positive value if white is better else it is a negative value
*/
int evaluationPosition(GameState state) {
    int colorToGoOg = state.colorToGo;
    // material
    int materialScore = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        int pieceValue = _pieceToValue(state.boardArray[i]);
        materialScore += pieceValue;
    }
    // mobility
    state.colorToGo = BLACK;
    Moves* blackLegalMoves = getValidMoves(&state, (GameStates){ .capacity = 0, .count = 0, .items = NULL });
    state.colorToGo = WHITE;
    Moves* whiteLegalMoves = getValidMoves(&state, (GameStates){ .capacity = 0, .count = 0, .items = NULL });
    int mobilityScore = (whiteLegalMoves->count - blackLegalMoves->count) * _mobilityWeight;
    state.colorToGo = colorToGoOg;
    free(whiteLegalMoves->items);
    free(whiteLegalMoves);
    free(blackLegalMoves->items);
    free(blackLegalMoves);
    return materialScore + mobilityScore;
}

void _orderMoves(Moves* moves, GameState state) {
    // TODO: Implement the move ordering
}

// TODO: Fix memory leaks in this function
int _search(int depth, int alpha, int beta, GameState currentState, GameStates *previousStates) {
    if (depth == 0) {
        int perspective = currentState.colorToGo == WHITE ? 1 : -1;
        // It changes the color to go but it is passed by value not reference so no side effects from evaluatePosition
        return evaluationPosition(currentState) * perspective;
    }
    Moves* moves = getValidMoves(&currentState, *previousStates);
    _orderMoves(moves, currentState);
    
    if (moves->count == 1) {
        int flag = moves->items[0] >> 12;
        if (flag == CHECKMATE) {
            free(moves->items);
            free(moves);
            return _negativeInfinity;
        } else if (flag == STALEMATE || flag == DRAW) {
            free(moves->items);
            free(moves);
            return 0;
        }
    }

    for (int i = 0; i < moves->count; i++) {
        // Making the opponent reponse move
        Move move = moves->items[i];
        GameState newState = copyState(currentState);
        makeMove(move, &newState); // New state is now the current state
        da_append(previousStates, currentState); // Current state is now the previous state
        int evaluation = _search(depth - 1, -beta, -alpha, newState, previousStates) * -1;
        
        previousStates->count--; // Removing previous state from the list
        free(newState.boardArray);

        if (evaluation >= beta) {
            // Pruning this branch
            free(moves->items);
            free(moves);
            return beta;
        }
        alpha = max(evaluation, alpha);
    }
    free(moves->items);
    free(moves);
    return alpha;
}

Moves bestMovesAccordingToComputer(int depth, GameState *state, GameStates *previousStates) {
    Moves* legalMoves;
    legalMoves = getValidMoves(state, *previousStates);

    int bestEval = _negativeInfinity;

    Moves bestMoves = {0};

    for (int i = 0; i < legalMoves->count; i++) {
        Move move = legalMoves->items[i];
        GameState newState = copyState(*state);
        makeMove(move, &newState);
        da_append(previousStates, *state);
        int eval = _search(depth, _negativeInfinity, _positiveInfinity,
            newState, previousStates) * -1;
        previousStates->count--;
        free(newState.boardArray);
        if (eval > bestEval) {
            bestEval = eval;
            bestMoves.count = 0;
            da_append(&bestMoves, move);
        } else if (eval == bestEval) {
            da_append(&bestMoves, move);
        }
    }
    free(legalMoves->items);
    free(legalMoves);
    return bestMoves;
}