#include <stdlib.h>
#include <string.h>

#include "PerftTranspositionTable.h"

#include "../LogChessStructs.h"

#define TRANSPOSITION_TABLE_SIZE_IN_BYTES (TRANSPOSITION_TABLE_SIZE_IN_MB * 1024 * 1024)
#define NB_ELEMENTS_IN_TRANSPOSITION_TABLE (TRANSPOSITION_TABLE_SIZE_IN_BYTES / sizeof(PerftTranspositionTable))

// This is 1 mb in size, which is quite a bit to be on the stack, so it is on the heap
PerftTranspositionTable* transpositionTable = NULL; 

bool PerftTranspositionTable_init() {
    if (transpositionTable != NULL) {
        // transposition table is already allocated
        return true;
    }
    transpositionTable = calloc(NB_ELEMENTS_IN_TRANSPOSITION_TABLE, sizeof(PerftTranspositionTable));
    return transpositionTable != NULL;
}

void PerftTranspositionTable_terminate() {
    if (transpositionTable != NULL) {
        free(transpositionTable);
    }
}

void PerftTranspositionTable_clear() {
    // We have to use the number of elements instead of the size in bytes to avoid writing beyond the table allocated memory
    memset(transpositionTable, 0, NB_ELEMENTS_IN_TRANSPOSITION_TABLE * sizeof(PerftTranspositionTable));
}

u64 PerftTranspositionTable_getPerftFromKey(ZobristKey key, u8 depth) {
    u32 index = (u32) (key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    PerftTranspositionTable entry = transpositionTable[index];
    
    // We are looking at a different position or at the same position with a different depth
    if (entry.key != key || entry.depth != depth) { return LOOKUP_FAILED; }

    return transpositionTable[index].perft;
}

void PerftTranspositionTable_recordPerft(PerftTranspositionTable entry) {
    u32 index = (u32) (entry.key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    memcpy(&transpositionTable[index], &entry, sizeof(PerftTranspositionTable));
}