#ifndef A2BCC12A_23A9_49F5_800A_DD78EE9CE23B
#define A2BCC12A_23A9_49F5_800A_DD78EE9CE23B

#include "GameState.h"

typedef struct {
    // Rank is already included in the side to move key
    u64 enPassantFile[9];
    // 16 total possibilities for castling
    u64 castlingPerms[16];
    // 14 is the biggest piece index ((Black pawn piece index - 9) + 1)
    // To access the random number for a piece for a square: pieces[piece - 9][square]
    u64 pieces[14][BOARD_SIZE];
    
    u64 sideToMove;
} ZobristRandomNumber;

ZobristRandomNumber* zobristRandomNumber;

void zobristKeyInitialize();
void zobristKeyTerminate();

/**
 * This function should be called only once when creating the your initial ChessGame struct
 * Subsequent Zobrist key will be calculated by the makeMove function incrementally
*/
void calculateZobristKey(ChessPosition* state);

#endif /* A2BCC12A_23A9_49F5_800A_DD78EE9CE23B */
