#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../UCICommandProcessing.h"
#include "../magicBitBoard/MagicBitBoard.h"
#include "../moveHandler/MoveGenerator.h"
#include "../moveHandler/MovePlayer.h"
#include "../utils/Math.h"

#include "Evaluation.h"
#include "RepetitionTable.h"
#include "TranspositionTable.h"
#include "MoveOrdering.h"
#include "Bot.h"

volatile bool endSearch = true;

ChessPosition currentPosition;

void Bot_provideGameStateForBot(ChessPosition state) {
    currentPosition = state;
}

int currentDepth;

u32 totalNodes;
u32 transpositionTableHits;

int quiescence(int alpha, int beta, int depth, Move* pv) {
    if (endSearch) return 0;

    // Standing pat
    // Negamax needs a relative evaluation, so positive means good for color to go and vice-versa
    int whoToMove = currentPosition.colorToGo == WHITE ? 1 : -1;
    int bestValue = Bot_staticEvaluation(currentPosition) * whoToMove;

    // If we are getting checkmated then we cannot play any moves so simply return the adjusted score
    if (bestValue == BOT_MINUS_INFINITY) return bestValue + depth;

    // The evaluation is better than our upper bound to we return staticEval (fail soft)
    if (bestValue >= beta) return bestValue;

    // If staticEval is better than the current lower bound it becomes that lower bound
    if (bestValue > alpha) alpha = bestValue;

    int nbOfMoves;
    Move captureMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    MoveHandler_getCaptures(captureMoves, &nbOfMoves, currentPosition);
    // Note: nbOfMoves is never equal to 0 because we already check
    // for this case in Bot_staticEvaluation by returning +-BOT_INFINITY
    // and we handle this case after calling Bot_staticEvaluation so we
    // are good

    Move childPv[MAXIMUM_DEPTH] = { 0 };
    ChessPosition previousPosition = currentPosition;

    for (int i = 0; i < nbOfMoves; i++) {
        Move move = captureMoves[i];
        MoveHandler_playMove(move, &currentPosition, true);

        int score = -quiescence(-beta, -alpha, depth + 1, childPv);

        currentPosition = previousPosition;
        RepetitionTable_pop();

        if (score >= beta) return score;
        if (score > bestValue) {
            bestValue = score;
            pv[0] = move;
            int pvIndex = 0;
            // Using null-terminated pv line
            while ((move = childPv[pvIndex++]) != NULL_MOVE) pv[pvIndex] = move;
        }
        if (score > alpha) alpha = score;
    }

    return bestValue;
}

int alpha_beta_negamax(int alpha, int beta, int depth, Move* pv, EntryType* type) {
    if (endSearch) return 0;

    totalNodes++;

    int startingAlpha = alpha;

    TranspositionTable_getPvLineFromKey(currentPosition.key, pv);
    int ttEval = TranspositionTable_getEvaluationFromKey(currentPosition.key, depth, alpha, beta, currentDepth - depth, type);
    if (ttEval != LOOKUP_FAILED) {
        transpositionTableHits++;
        return ttEval;
    }

    if (depth == 0) {
        *type = EXACT;
        return quiescence(alpha, beta, currentDepth, pv);
    }

    int bestEval = BOT_MINUS_INFINITY;
    EntryType bestType = EXACT;
    EntryType childType;
    Move childPv[MAXIMUM_DEPTH] = { 0 };

    int nbOfMoves;
    Move validMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    MoveHandler_getValidMoves(validMoves, &nbOfMoves, currentPosition);

    // If we have no moves, return the appropriate score
    if (nbOfMoves == 0) {
        *type = EXACT;
        // Adjust the score if we are getting mated
        if (MoveHandler_isKingInCheck()) bestEval = BOT_MINUS_INFINITY + (currentDepth - depth);
        // If its a draw return 0
        else bestEval = 0;
        goto end_depth_search;
    }

    MoveOrdering_orderMoves(validMoves, nbOfMoves, pv[0], currentPosition.board);

    ChessPosition previousPosition = currentPosition;

    for (int i = 0; i < nbOfMoves; i++) {
        Move move = validMoves[i];
        MoveHandler_playMove(move, &currentPosition, true);

        // We switch alpha and beta, because alpha is the lower bound for the color to play 
        // but it is the upper bound for the other color. Opposite is true for beta
        // so we need to switch them we it is the opposite color to play
        int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, childPv, &childType);

        currentPosition = previousPosition;
        RepetitionTable_pop();

        // Update the best evaluation that we found
        if (eval >= bestEval) {
            // Updating score, score type and pv move 
            bestEval = eval;
            bestType = childType;
            pv[0] = move;
            int pvIndex = 0;
            // Using null-terminated pv line
            while ((move = childPv[pvIndex++]) != NULL_MOVE) pv[pvIndex] = move;
        }

        // Update the best move that we found from the previous depth
        // or this depth if we have already updated alpha during this depth search
        if (eval > alpha) alpha = eval;

        // Fail-soft beta cutoff: This move is too good, our opponent will not allow it
        if (eval >= beta) break;

        if (endSearch) return 0;
    }

end_depth_search:
    if (bestEval <= startingAlpha) *type = UPPER_BOUND;
    else if (bestEval >= beta) *type = LOWER_BOUND;
    else *type = bestType;

    TranspositionTable_recordEntry(currentPosition.key, depth, *type, bestEval, currentDepth - depth, pv, depth);

    return bestEval;
}

Move Bot_think() {
    char* pvString = calloc(7 * sizeof(char), MAXIMUM_DEPTH);
    char evalTypeString[13];

    Move principalVariations[MAXIMUM_DEPTH] = { 0 };
    Move tempPV[MAXIMUM_DEPTH] = { 0 };
    int bestEvalFromCurrentSearch;
    EntryType bestEvalType;
    EntryType evalType;

    Move rootMoves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES];
    int nbMoves;
    MoveHandler_getValidMoves(rootMoves, &nbMoves, currentPosition);

    // The first element is the root position
    ChessPosition startingPosition = currentPosition;

    bool isMate;
    int numPlies;

    for (currentDepth = 1; currentDepth < MAXIMUM_DEPTH; currentDepth++) {
        // At every depth, we need to reset the bestEvalFromSearch to BOT_MINUS_INFINITY
        // because the result from a lower depth are irrelevant when searching at a higher depth
        bestEvalFromCurrentSearch = BOT_MINUS_INFINITY;

        // values for uci info command
        totalNodes = 0;
        transpositionTableHits = 0;

        MoveOrdering_orderMoves(rootMoves, nbMoves, principalVariations[0], currentPosition.board);

        // Check endSearch before the long search loop
        if (endSearch) goto stop_search;

        for (int index = 0; index < nbMoves; index++) {
            Move rootMove = rootMoves[index];
            MoveHandler_playMove(rootMove, &currentPosition, true);

            // We input the depth minus 1 because we already make one move with the root negamax loop
            // We need to do the negative of negamax because a good score for our opponent is a bad score for us
            int score = -alpha_beta_negamax(BOT_MINUS_INFINITY, BOT_INFINITY, currentDepth - 1, tempPV, &evalType);

            currentPosition = startingPosition;
            RepetitionTable_pop();

            // Check endSearch before using the calculated score because it could be 0 
            // if endSearch is triggered when we were searching
            if (endSearch) goto stop_search;

            if (score >= bestEvalFromCurrentSearch) {
                bestEvalFromCurrentSearch = score;
                bestEvalType = evalType;

                principalVariations[0] = rootMove;
                // It is mate
                isMate = abs(bestEvalFromCurrentSearch) >= BOT_INFINITY - MAX_MATE_DEPTH;
                if (isMate) numPlies = bestEvalFromCurrentSearch < 0 ? BOT_INFINITY + bestEvalFromCurrentSearch : BOT_INFINITY - bestEvalFromCurrentSearch;
                else numPlies = currentDepth - 1;
                // printf("isMate: %d, score: %d, numPlies: %d\n", isMate, score, numPlies);

                memcpy(&principalVariations[1], tempPV, numPlies * sizeof(Move));
            }

        }

        int infoScore;
        char scoreUnits[5];
        if (!isMate) {
            strcpy(scoreUnits, "cp");
            int view = currentPosition.colorToGo == WHITE ? 1 : -1;
            infoScore = bestEvalFromCurrentSearch * view;
        }
        else {
            // We have found a mate!
            strcpy(scoreUnits, "mate");
            // Extracting the depth it was found
            numPlies = bestEvalFromCurrentSearch < 0 ? BOT_MINUS_INFINITY - bestEvalFromCurrentSearch : BOT_INFINITY - bestEvalFromCurrentSearch;
            infoScore = numPlies / 2 + numPlies % 2;
        }

        int charIndex = 0;
        int bound = isMate ? numPlies : currentDepth;
        for (int index = 0; index < bound; index++) {
            Move pv = principalVariations[index];
            if (pv == NULL_MOVE) break; // Just to be sure we aren't sending garbage
            charIndex += string_moveToLongAlgebraic(pv, pvString + charIndex);
            pvString[charIndex++] = ' ';
        }
        pvString[charIndex] = '\0';

        switch (bestEvalType) {
        case EXACT: strcpy(evalTypeString, " "); break;
        case LOWER_BOUND: strcpy(evalTypeString, " lowerbound "); break;
        case UPPER_BOUND: strcpy(evalTypeString, " upperbound "); break;
        }

        UCI_sendResponse(
            "info depth %d nodes %u tbhits %u score %s %d%spv %s\n",
            currentDepth,
            totalNodes,
            transpositionTableHits,
            scoreUnits,
            infoScore,
            evalTypeString,
            pvString
        );

        // If we have found a mate with a type EXACT or
        // we are giving mate (sign(bestEvalFromCurrentSearch) == 1) and type is LOWER_BOUND or
        // we are getting mated (sign(bestEvalFromCurrentSearch) == -1) and type is UPPER_BOUND
        if (isMate && (
            (evalType == EXACT) ||
            (sign(bestEvalFromCurrentSearch) == 1 && evalType == LOWER_BOUND) ||
            (sign(bestEvalFromCurrentSearch) == -1 && evalType == UPPER_BOUND)
            )) {
            goto wait_search;
        }

        if (endSearch) goto stop_search;
    }

wait_search:
    while (!endSearch);

stop_search:
    free(pvString);
    return principalVariations[0];
}

// Standard time fraction of 2.5%
#define TIME_FRACTION (40)

u64 Bot_calculateThinkTime(u64 whiteTime, u64 blackTime, u64 whiteInc, u64 blackInc, bool isWhiteToMove) {
    u64 myTime = isWhiteToMove ? whiteTime : blackTime;
    u64 myInc = isWhiteToMove ? whiteInc : blackInc;

    u64 baseTime = myTime / TIME_FRACTION;
    u64 bonusTime = myInc >> 1;

    u64 thinkTime = baseTime + bonusTime;

    // Clamp to a minimum to prevent 0ms think time
    // The division by 4 is from a Sebastian Lague video
    if (thinkTime >> 2 < BOT_DEFAULT_THINK_TIME_MS) thinkTime = BOT_DEFAULT_THINK_TIME_MS;

    return thinkTime;
}
