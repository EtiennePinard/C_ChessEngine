#ifndef A267B67F_4ECE_456A_A4EB_C6E73ACF11E0
#define A267B67F_4ECE_456A_A4EB_C6E73ACF11E0

#include "../utils/Utils.h"

/**
* @brief The structure is copied from Sebastian Lague chess program
* A move is a 16 bit number
* bit 0-5: from square (0 to 63)
* bit 6-11: to square (0 to 63)
* bit 12-15: flag
*/
typedef u16 Move;

/**
 * @brief A flag is a 4 bit number which tells additional information
 * about move, e.g. that it leads to a promotion or a checkmate.
*/
typedef enum {
    NOFLAG, 
    EN_PASSANT, 
    DOUBLE_PAWN_PUSH, 
    KING_SIDE_CASTLING, 
    QUEEN_SIDE_CASTLING, 
    PROMOTE_TO_QUEEN, 
    PROMOTE_TO_KNIGHT, 
    PROMOTE_TO_ROOK, 
    PROMOTE_TO_BISHOP
} Flag;

#define Move_fromSquare(move) (move & 0b111111)
#define Move_toSquare(move) ((move >> 6) & 0b111111)
#define Move_flag(move) ((Flag) (move >> 12))
#define Move_makeMove(from, to, flag) (Move) (from + (to << 6) + (flag << 12))

#define NULL_MOVE ((Move) 0)

#endif /* A267B67F_4ECE_456A_A4EB_C6E73ACF11E0 */
