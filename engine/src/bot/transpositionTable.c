#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "Bot.h"
#include "TranspositionTable.h"

#define TRANSPOSITION_TABLE_SIZE_IN_BYTES (TRANSPOSITION_TABLE_SIZE_IN_MB * 1024 * 1024)
#define NB_ELEMENTS_IN_TRANSPOSITION_TABLE (TRANSPOSITION_TABLE_SIZE_IN_BYTES / sizeof(TTEntry))

// This is 1 mb in size, which is quite a bit to be on the stack, so it is on the heap
TTEntry* transpositionTable = NULL;

bool TranspositionTable_init() {
    assert(transpositionTable == NULL);

    transpositionTable = calloc(NB_ELEMENTS_IN_TRANSPOSITION_TABLE, sizeof(TTEntry));
    return transpositionTable != NULL;
}

void TranspositionTable_terminate() {
    assert(transpositionTable != NULL);

    TranspositionTable_clear();

    free(transpositionTable);
    transpositionTable = NULL;
}

void TranspositionTable_clear() {
    assert(transpositionTable != NULL);

    for (size_t i = 0; i < NB_ELEMENTS_IN_TRANSPOSITION_TABLE; i++) {
        if (transpositionTable[i].pvLine != NULL) {
            free(transpositionTable[i].pvLine);
            transpositionTable[i].pvLine = NULL;
        }
    }

    memset(transpositionTable, 0, TRANSPOSITION_TABLE_SIZE_IN_BYTES);
}

void TranspositionTable_getPvLineFromKey(ZobristKey key, Move* pvLine) {
    assert(transpositionTable != NULL);

    TTEntry entry = transpositionTable[key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE];

    if (entry.type == EXACT && entry.pvLength > 0) {
        memcpy(pvLine, entry.pvLine, sizeof(Move) * entry.pvLength);
    }
}

int TranspositionTable_getEvaluationFromKey(ZobristKey key, int depth, int alpha, int beta, int plyFromRoot, EntryType* type) {
    assert(transpositionTable != NULL);

    TTEntry entry = transpositionTable[key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE];

    // We are looking at a different position or at the same position with a higher depth, 
    // which makes the evaluation at a lower depth not that useful to us
    if (entry.key != key || entry.depth < depth) return LOOKUP_FAILED;

    *type = entry.type;

    int eval = entry.evaluation;

    // Unpack mate scores
    if (eval > BOT_INFINITY - MAX_MATE_DEPTH) {
        eval += plyFromRoot;
    }
    else if (eval < BOT_MINUS_INFINITY + MAX_MATE_DEPTH) {
        eval -= plyFromRoot;
    }

    if (entry.type == EXACT) return entry.evaluation;
    if (entry.type == LOWER_BOUND && entry.evaluation <= alpha) return alpha;
    if (entry.type == UPPER_BOUND && entry.evaluation >= beta) return beta;

    return LOOKUP_FAILED;
}

void TranspositionTable_recordEntry(
    ZobristKey key,
    u8 depth,
    EntryType type,
    int evaluation,
    int plyFromRoot,
    Move* pvLine,
    int pvLength
) {
    assert(transpositionTable != NULL);

    TTEntry* entry = &transpositionTable[key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE];

    entry->key = key;
    entry->depth = depth;
    entry->type = type;
    entry->evaluation = evaluation;

    // Adjust mate score
    if (evaluation > BOT_INFINITY - MAX_MATE_DEPTH) entry->evaluation -= plyFromRoot;
    else if (evaluation < BOT_MINUS_INFINITY + MAX_MATE_DEPTH) entry->evaluation += plyFromRoot;

    if (entry->pvLine != NULL) {
        free(entry->pvLine);
        entry->pvLine = NULL;
    }

    if (type == EXACT && pvLength > 0) {
        entry->pvLine = malloc(sizeof(Move) * pvLength);
        if (!entry->pvLine) {
            fprintf(stderr, "Failed to allocate memory for pvLine\n");
            exit(EXIT_FAILURE);
        }
        memcpy(entry->pvLine, pvLine, sizeof(Move) * pvLength);
        entry->pvLength = pvLength;
    }
    else {
        entry->pvLength = 0;
    }
}
