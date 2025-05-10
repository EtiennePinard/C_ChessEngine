#include "ZobristKey.h"
#include <time.h>
#include <assert.h>
#include <stdlib.h>

// Total size of the struct is 57.6 kb, which is a lot but not too bad 
ZobristRandomNumber zobristRandomNumber = (ZobristRandomNumber) { 0 };

/**
 * @brief Returns a pseudo-random unsigned 64 bit number. 
 * This function uses rand() to generate pseudo-random numbers
 * so make sure to set the random seed before calling this function.
 * 
 * @return u64 A pseudo-random 64 bit number
 */
u64 random_u64() {
    u64 u1 = (u64)(rand()) & 0xFFFF; 
    u64 u2 = (u64)(rand()) & 0xFFFF;
    u64 u3 = (u64)(rand()) & 0xFFFF; 
    u64 u4 = (u64)(rand()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
  }

bool ZobristKey_init() {
    srand(time(NULL));

    for (PieceCharacteristics pieceColor = WHITE; pieceColor <= BLACK; pieceColor += 8) {
        // Piece order influenced
        for (PieceCharacteristics piece = PAWN; piece <= KING; piece++) {
            Piece index = Piece_makePiece(pieceColor, piece) - 9;
            for (int square = 0; square < BOARD_SIZE; square++) {
                zobristRandomNumber.pieces[(int) index][square] = random_u64();
            }
        }
    }
    for (int castlingPermStates = 0; castlingPermStates < 16; castlingPermStates++) {
        zobristRandomNumber.castlingPerms[castlingPermStates] = random_u64();
    }
    for (int i = 0; i < 9; i++) {
        zobristRandomNumber.enPassantFile[i] = i == 0 ? (u64) 0 : random_u64();
    }
    zobristRandomNumber.sideToMove = random_u64();

    return true;
}

// Only use this function to set the initial Zobrist key
// The subsequent keys are computed incrementally by the playMove function
void ZobristKey_calculateInitialKey(ChessPosition* state) {
    assert(state != NULL);

    u64 zobristKey = (u64) 0;

    for (int square = 0; square < BOARD_SIZE; square++) {
        Piece piece = Board_pieceAtIndex(state->board, square);
        if (piece != NOPIECE) {
            zobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][square];
        }
    }

    zobristKey ^= zobristRandomNumber.enPassantFile[state->enPassantTargetSquare];

    if (state->colorToGo == WHITE) {
        zobristKey ^= zobristRandomNumber.sideToMove;
    }

    zobristKey ^= zobristRandomNumber.castlingPerms[(int) state->castlingPerm];

    state->key = zobristKey;
}