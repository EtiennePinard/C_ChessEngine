#ifndef A267B67F_4ECE_456A_A4EB_C6E73ACF11E0
#define A267B67F_4ECE_456A_A4EB_C6E73ACF11E0

#include "Constants.h"

/**
* The structure is copied from Sebastian Lague chess program
* A move is a 16 bit number
* bit 0-5: from square (0 to 63)
* bit 6-11: to square (0 to 63)
* bit 12-15: flag
*/
typedef unsigned short Move;

/**
 * A flag is a 4 bit number which tells additional information
 * about move, e.g. that it leads to a promotion or a checkmate.
*/
typedef enum {
    NOFlAG, 
    EN_PASSANT, 
    DOUBLE_PAWN_PUSH, 
    KING_SIDE_CASTLING, 
    QUEEN_SIDE_CASTLING, 
    PROMOTE_TO_QUEEN, 
    PROMOTE_TO_KNIGHT, 
    PROMOTE_TO_ROOK, 
    PROMOTE_TO_BISHOP, 
    STALEMATE, 
    CHECKMATE, 
    DRAW 
} Flag;

char fromSquareFromMove(Move move);
char toSquareFromMove(Move move);
Flag flagFromMove(Move move);

Move createMove(int from, int to, Flag flag);

int nbMovesInArray(Move moves[MAX_LEGAL_MOVES + 1]);

#endif /* A267B67F_4ECE_456A_A4EB_C6E73ACF11E0 */
