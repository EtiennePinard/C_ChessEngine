#ifndef A267B67F_4ECE_456A_A4EB_C6E73ACF11E0
#define A267B67F_4ECE_456A_A4EB_C6E73ACF11E0

#include "../utils/Utils.h"

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
    PROMOTE_TO_BISHOP
} Flag;

#define fromSquare(move) (move & 0b111111)
#define toSquare(move) ((move >> 6) & 0b111111)
#define flagFromMove(move) ((Flag) (move >> 12))
#define makeMove(from, to, flag) (from + (to << 6) + (flag << 12))

#endif /* A267B67F_4ECE_456A_A4EB_C6E73ACF11E0 */
