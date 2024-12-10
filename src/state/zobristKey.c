#include "ZobristKey.h"
#include <time.h>
#include <assert.h>
#include <stdlib.h>

// Total size of the struct is 57.6 kb, which is a lot but not too bad 
ZobristRandomNumber zobristRandomNumber = (ZobristRandomNumber) { 0 };

void zobristKeyInitialize() {

    srand(time(NULL));
    // Don't need to check if already initialized because we are calling this function from our own program
    // Other programs will not (at least should not) call this function

    for (int color = 0; color < 2; color++) {
        PieceCharacteristics pieceColor = WHITE * (color + 1);
        // Piece order influenced
        for (PieceCharacteristics piece = PAWN; piece <= KING; piece++) {
            Piece index = makePiece(pieceColor, piece) - 9;
            for (int square = 0; square < BOARD_SIZE; square++) {
                zobristRandomNumber.pieces[(int) index][square] = random_u64();
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        zobristRandomNumber.castlingPerms[i] = random_u64();
    }
    for (int i = 0; i < 9; i++) {
        zobristRandomNumber.enPassantFile[i] = i == 0 ? (u64) 0 : random_u64();
    }
    zobristRandomNumber.sideToMove = random_u64();
}

// Only use this function to set the initial Zobrist key
// The subsequent keys are computed incrementally by the makeMove function
void calculateZobristKey(ChessPosition* state) {
    u64 zobristKey = (u64) 0;

    for (int square = 0; square < BOARD_SIZE; square++) {
        Piece piece = pieceAtIndex(state->board, square);
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