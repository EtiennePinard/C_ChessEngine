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
    memset(transpositionTable, 0, TRANSPOSITION_TABLE_SIZE_IN_BYTES);
}

u64 PerftTranspositionTable_getPerftFromKey(
    ZobristKey key, u8 depth 
    #ifdef DEBUG
    , ChessPosition pos
    #endif
    ) {
    u32 index = (u32) (key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    PerftTranspositionTable entry = transpositionTable[index];
    
    // We are looking at a different position or at the same position with a different depth
    if (entry.key != key || entry.depth != depth) { return LOOKUP_FAILED; }

    #ifdef DEBUG
    if (entry.chessPosition.castlingPerm != pos.castlingPerm) {
        printf("The castling perm %d and %d are not equal!\n", entry.chessPosition.castlingPerm, pos.castlingPerm);
    } else if (entry.chessPosition.colorToGo != pos.colorToGo) {
        printf("The color to go %d and %d are not equal!\n", entry.chessPosition.colorToGo, pos.colorToGo);
    } else if (memcmp(entry.chessPosition.board.bitboards, pos.board.bitboards, sizeof(u64) * 14) != 0) {
        printf("The board of the two boards are not equal!\n");
        printBoard(entry.chessPosition.board);
        printBoard(pos.board);
    } else if (entry.chessPosition.enPassantTargetSquare != pos.enPassantTargetSquare) {
        printf("The en passant square %d and %d are not equal!\n", entry.chessPosition.enPassantTargetSquare, pos.enPassantTargetSquare);
    } else {
        return transpositionTable[index].perft;
    }

    printf("THERE IS A COLLISION :(\n");
    printBoard(entry.chessPosition.board);
    printBoard(pos.board);
    exit(EXIT_FAILURE);
    
    #endif

    return transpositionTable[index].perft;
}

void PerftTranspositionTable_recordPerft(PerftTranspositionTable entry) {
    u32 index = (u32) (entry.key % NB_ELEMENTS_IN_TRANSPOSITION_TABLE);
    memcpy(&transpositionTable[index], &entry, sizeof(PerftTranspositionTable));
}