#include <stdlib.h>
#include <string.h>

#include "TranspositionTable.h"

#define TRANSPOSITION_TABLE_SIZE_IN_BYTES (TRANSPOSITION_TABLE_SIZE_IN_MB * 1024 * 1024)
#define NB_ELEMENTS_IN_TRANSPOSITION_TABLE (TRANSPOSITION_TABLE_SIZE_IN_BYTES / sizeof(TranspositionTable))

// This is 1 mb in size, which is quite a bit to be on the stack, so it is on the heap
TranspositionTable* transpositionTable = NULL; 

bool TTable_init() {
    if (transpositionTable != NULL) {
        // transposition table is already allocated
        return true;
    }
    transpositionTable = calloc(NB_ELEMENTS_IN_TRANSPOSITION_TABLE, sizeof(TranspositionTable));
    return transpositionTable != NULL;
}

void TTable_terminate() {
    if (transpositionTable != NULL) {
        free(transpositionTable);
    }
}

void TTable_clear() {
    memset(transpositionTable, 0, TRANSPOSITION_TABLE_SIZE_IN_BYTES);
}

Move TTable_getMoveFromKey(ZobristKey key) {
    u32 index = (u32) (key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    return transpositionTable[index].move;
}