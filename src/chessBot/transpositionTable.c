#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "TranspositionTable.h"

#define TRANSPOSITION_TABLE_SIZE_IN_BYTES (TRANSPOSITION_TABLE_SIZE_IN_MB * 1024 * 1024)
#define NB_ELEMENTS_IN_TRANSPOSITION_TABLE (TRANSPOSITION_TABLE_SIZE_IN_BYTES / sizeof(TranspositionTable))

// This is 1 mb in size, which is quite a bit to be on the stack, so it is on the heap
TranspositionTable* transpositionTable = NULL; 

bool TranspositionTable_init() {
    assert(transpositionTable == NULL);

    transpositionTable = calloc(NB_ELEMENTS_IN_TRANSPOSITION_TABLE, sizeof(TranspositionTable));
    return transpositionTable != NULL;
}

void TranspositionTable_terminate() {
    assert(transpositionTable != NULL);

    free(transpositionTable);
    transpositionTable = NULL;
}

void TranspositionTable_clear() {
    assert(transpositionTable != NULL);

    memset(transpositionTable, 0, TRANSPOSITION_TABLE_SIZE_IN_BYTES);
}

Move TranspositionTable_getMoveFromKey(ZobristKey key) {
    assert(transpositionTable != NULL);

    u32 index = (u32) (key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    
    return transpositionTable[index].bestMove;
}

int TranspositionTable_getEvaluationFromKey(ZobristKey key, int depth, int alpha, int beta) {
    assert(transpositionTable != NULL);

    TranspositionTable entry = transpositionTable[key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE];

    // We are looking at a different position or at the same position with a higher depth, 
    // which makes the evaluation at a lower depth not that useful to us
    if (entry.key != key || entry.depth < depth) { return LOOKUP_FAILED; }

    // TODO: See if we need to correct the score for mates
    if (entry.type == EXACT) { return entry.evaluation; }
    if (entry.type == LOWER_BOUND && entry.evaluation <= alpha) { return alpha; }
    if (entry.type == UPPER_BOUND && entry.evaluation >= beta) { return beta; }
    
    return LOOKUP_FAILED;
}

void TranspositionTable_recordEntry(ZobristKey key, u8 depth, EntryType type, Move move, int evaluation) {
    assert(transpositionTable != NULL);
    
    u32 index = (u32) (key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    
    transpositionTable[index].key = key;
    transpositionTable[index].depth = depth;
    transpositionTable[index].type = type;
    transpositionTable[index].bestMove = move;
    transpositionTable[index].evaluation = evaluation;
}
