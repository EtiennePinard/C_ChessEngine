#include <assert.h>
#include <stdlib.h>
#include "chessComputer.h"
#include "chessGameEmulator.h"
#include "../testing/logChessStructs.h"

#define _pawnValue 100
#define _knightValue 300
#define _bishopValue 300
#define _rookValue 500
#define _queenValue 900

#define _mobilityWeight 1 // Mobility is not that important

#define _positiveInfinity 9999999
#define _negativeInfinity -9999999

#define _capturedPieceValueMultiplier 10

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
    Moves* blackLegalMoves = getValidMoves(&state, NULL);
    state.colorToGo = WHITE;
    Moves* whiteLegalMoves = getValidMoves(&state, NULL);
    int mobilityScore = (whiteLegalMoves->count - blackLegalMoves->count) * _mobilityWeight;
    state.colorToGo = colorToGoOg;
    free(whiteLegalMoves->items);
    free(whiteLegalMoves);
    free(blackLegalMoves->items);
    free(blackLegalMoves);
    return materialScore + mobilityScore;
}

void _orderMoves(Moves* moves, GameState state) {
    // Giving each move a score
    int scores[moves->count];
    for (int i = 0; i < moves->count; i++) {
        Move move = moves->items[i];
        int score = 0;
        int startSquare = move & 0b111111;
        int endSquare = (move >> 6) & 0b111111;
        int pieceThatMoves = state.boardArray[startSquare];
        int pieceToCapture = state.boardArray[endSquare];

        // Captures a piece with a smaller piece
        if ((pieceToCapture & pieceTypeBitMask) != NONE) {
            score += _capturedPieceValueMultiplier * abs(_pieceToValue(pieceToCapture)) - 
                abs(_pieceToValue(pieceThatMoves));
        }

        // Promote a piece
        if ((pieceToCapture & pieceTypeBitMask) == PAWN) {
            int flag = move >> 12;
            switch (flag) {
            case PROMOTE_TO_QUEEN:
                score += _queenValue;
                break;
            case PROMOTE_TO_KNIGHT:
                score += _knightValue;
                break;
            case PROMOTE_TO_ROOK:
                score += _rookValue;
                break;
            case PROMOTE_TO_BISHOP:
                score += _bishopValue;
                break;
            default:
                break;
            }

            scores[i] = score;
        }

        // Sorting Moves
        int order[moves->count];
        int tempScores[moves->count];
        int lastSmallestIndex = 0;
        for (int i = 0; i < moves->count; i++) {
            // Removing any score that was already sorted
            for (int x = 0; x < moves->count - i; x++) {
                int score = scores[x];
                if (score != -1) {
                    tempScores[x] = score;
                }
            }
            // Sorting the remainging scores
            int nextSmall = tempScores[0];
            for (int x = 1; x < moves->count - i; x++) {
                int score = tempScores[x];
                if (score < nextSmall) {
                    nextSmall = score;
                    lastSmallestIndex = x;
                }
            }
            order[i] = lastSmallestIndex;
            scores[lastSmallestIndex] = -1;
        }

        // Ordering the original moves dynamic array
        Move results[moves->count];
        for (int i = 0; i < moves->count; i++) {
            results[i] = moves->items[order[i]]; 
        }
        for (int i = 0; i < moves->count; i++) {
            moves->items[i] = results[i];
        }
    }
}

int _search(int depth, int alpha, int beta, GameState currentState, GameStates *previousStates) {
    if (depth == 0) {
        int perspective = currentState.colorToGo == WHITE ? 1 : -1;
        // It changes the color to go but it is passed by value not reference so no side effects from evaluatePosition
        return evaluationPosition(currentState) * perspective;
    }
    Moves* moves = getValidMoves(&currentState, previousStates);
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

Moves* bestMovesAccordingToComputer(int depth, const GameState *state, GameStates *previousStates) {
    Moves* legalMoves;
    legalMoves = getValidMoves(state, previousStates);

    int bestEval = _negativeInfinity;

    Moves* bestMoves = malloc(sizeof(Moves));
    bestMoves->capacity = 0;
    bestMoves->count = 0;
    bestMoves->items = NULL;

    for (int i = 0; i < legalMoves->count; i++) {
        Move move = legalMoves->items[i];
        GameState newState = copyState(*state);
        makeMove(move, &newState);
        da_append(previousStates, *state);
        int eval = _search(depth, _negativeInfinity, _positiveInfinity,
            newState, previousStates) * -1;
        if (previousStates->count > 0) {
            previousStates->count--;
        }
        free(newState.boardArray);
        if (eval > bestEval) {
            bestEval = eval;
            bestMoves->count = 0;
            da_append(bestMoves, move);
        } else if (eval == bestEval) {
            da_append(bestMoves, move);
        }
    }
    free(legalMoves->items);
    free(legalMoves);

    return bestMoves;
}