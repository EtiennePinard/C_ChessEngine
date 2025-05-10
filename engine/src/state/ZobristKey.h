#ifndef A2BCC12A_23A9_49F5_800A_DD78EE9CE23B
#define A2BCC12A_23A9_49F5_800A_DD78EE9CE23B

#include <stdbool.h>
#include "Piece.h"
#include "GameState.h"

typedef struct ZobristRandomNumber {
    // Rank is already included in the side to move key
    u64 enPassantFile[9];
    // 16 total possibilities for castling
    u64 castlingPerms[16];
    // 14 is the biggest piece index ((Black pawn piece index - 9) + 1)
    // To access the random number for a piece for a square: pieces[piece - 9][square]
    u64 pieces[MAX_PIECE_INDEX - 9 + 1][BOARD_SIZE];
    
    u64 sideToMove;
} ZobristRandomNumber;

extern ZobristRandomNumber zobristRandomNumber;

/**
 * @brief Initializes the zobrist key random numbers
 * 
 * @return true If the zobrist key were initialize properly
 * @return false If the zobrist key could not be initialize properly
 */
bool ZobristKey_init();

/**
 * @brief Calculates the zobrist key for a given chess position and stores it in the position
 * Note that this function should be only be called when you first create you chess position.
 * The subsequent keys are create incrementally when you play a move
 * 
 * @param position The position to calculate the zobrist key of and store it in it
 */
void ZobristKey_calculateInitialKey(ChessPosition* position);

#endif /* A2BCC12A_23A9_49F5_800A_DD78EE9CE23B */
