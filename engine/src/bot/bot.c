#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../magicBitBoard/MagicBitBoard.h"
#include "../moveHandler/MoveGenerator.h"
#include "../moveHandler/MovePlayer.h"
#include "../utils/Math.h"

#include "Evaluation.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "MoveOrdering.h"
#include "Bot.h"

volatile bool endSearch = false;

ChessPosition currentPosition;

void Bot_provideGameStateForBot(ChessPosition state) {
    currentPosition = state;
}

// Max depth is 100 for now
#define MAXIMUM_DEPTH (100)
ChessPosition posHistory[MAXIMUM_DEPTH];
int currentDepth;

Move bestMoveFromCurrentDepthSearch;
int bestEvalFromCurrentSearch;

Move principalVariation;

u32 totalNodes;
u32 leafNodes;
u32 transpositionTableHits;

int alpha_beta_negamax(int alpha, int beta, int depth) {
    if (endSearch) return 0;

    totalNodes++;

    int startingAlpha = alpha;

    int ttEval = TranspositionTable_getEvaluationFromKey(currentPosition.key, depth, alpha, beta);
    if (ttEval != LOOKUP_FAILED) {
        transpositionTableHits++;
        return ttEval;
    }

    if (depth == 0) {
        leafNodes++;
        // Negamax needs a relative evaluation, so positive means good for color to go and vice-versa
        int whoToMove = currentPosition.colorToGo == WHITE ? 1 : -1;
        int evaluation = Bot_staticEvaluation(currentPosition) * whoToMove;
        return evaluation;
    }

    // The best move from this depth is the principal variation at this depth
    Move bestMove = NULL_MOVE;
    int bestEval = BOT_MINUS_INFINITY;

    int nbOfMoves;
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    MoveHandler_getValidMoves(validMoves, &nbOfMoves, currentPosition);

    Move ttMove = TranspositionTable_getMoveFromKey(currentPosition.key);
    MoveOrdering_orderMoves(validMoves, nbOfMoves, ttMove, currentPosition.board);

    posHistory[currentDepth - depth] = currentPosition;

    for (int i = 0; i < nbOfMoves; i++) {
        Move move = validMoves[i];
        MoveHandler_playMove(move, &currentPosition, true);

        // We switch alpha and beta, because alpha is the lower bound for the color to play 
        // but it is the upper bound for the other color. Opposite is true for beta
        // so we need to switch them we it is the opposite color to play
        int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1);

        memcpy(&currentPosition, &posHistory[currentDepth - depth], sizeof(ChessPosition));
        RepetitionTable_pop();
        
        // Update the best evaluation that we found
        if (eval > bestEval) {
            bestMove = move;
            bestEval = eval;
        }

        // Update the best move that we found from the previous depth
        // or this depth if we have already updated alpha during this depth search
        if (eval > alpha) alpha = eval;

        // Fail-soft beta cutoff: This move is too good, our opponent will not allow it
        if (eval >= beta) break;

        if (endSearch) return 0;
    }

    EntryType type;
    if (bestEval <= startingAlpha) type = UPPER_BOUND;
    else if (bestEval >= beta) type = LOWER_BOUND;
    else type = EXACT;

    TranspositionTable_recordEntry(currentPosition.key, currentDepth, type, bestMove, bestEval);

    return bestEval;
}

Move Bot_think() {
    bestMoveFromCurrentDepthSearch = NULL_MOVE;
    principalVariation = NULL_MOVE; // aka best move from full depth search

    Move rootMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int nbMoves;
    MoveHandler_getValidMoves(rootMoves, &nbMoves, currentPosition);

    // The first element is the root position
    posHistory[0] = currentPosition;

    for (int depth = 1; depth < MAXIMUM_DEPTH; depth++) {
        currentDepth = depth;
        // At every depth, we need to reset the bestEvalFromSearch to BOT_MINUS_INFINITY
        // because the result from a lower depth are irrelevant when searching at a higher depth
        bestEvalFromCurrentSearch = BOT_MINUS_INFINITY;

        // debug information
        totalNodes = 0;
        leafNodes = 0;
        transpositionTableHits = 0;

        MoveOrdering_orderMoves(rootMoves, nbMoves, principalVariation, currentPosition.board);

        // Check endSearch before the long search loop
        if (endSearch) goto stop_search;

        for (int index = 0; index < nbMoves; index++) {
            Move move = rootMoves[index];
            MoveHandler_playMove(move, &currentPosition, true);

            // We input the depth minus 1 because we already make one move with the root negamax loop
            // We need to do the negative of negamax because a good score for our opponent is a bad score for us
            int score = -alpha_beta_negamax(BOT_MINUS_INFINITY, BOT_INFINITY, depth - 1);

            // Check endSearch before using the calculated score because it could be 0 
            // if endSearch is triggered when we were searching
            if (endSearch) goto stop_search;


            if (score > bestEvalFromCurrentSearch) {
                bestEvalFromCurrentSearch = score;
                bestMoveFromCurrentDepthSearch = move;
            }

            memcpy(&currentPosition, posHistory, sizeof(ChessPosition));
            RepetitionTable_pop();
        }

        // We have done one full depth search and so we update the full search best move
        principalVariation = bestMoveFromCurrentDepthSearch;

        // Added this print statement to make it more convenient when debugging the bot
        printf("Depth %d search finished, %u leaf nodes, %u total nodes, %u ttHits, %d best eval, %c%d%c%d best move\n",
            depth,
            leafNodes,
            totalNodes,
            transpositionTableHits,
            bestEvalFromCurrentSearch,
            'a' + file(Move_fromSquare(principalVariation)),
            8 - rank(Move_fromSquare(principalVariation)),
            'a' + file(Move_toSquare(principalVariation)),
            8 - rank(Move_toSquare(principalVariation))
        );

        if (endSearch) goto stop_search;
    }

stop_search:
    return principalVariation;
}
