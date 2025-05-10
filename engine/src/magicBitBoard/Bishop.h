#ifndef BFF9D0FF_89E9_404D_BAB7_475DDDC1578B
#define BFF9D0FF_89E9_404D_BAB7_475DDDC1578B

#include "../utils/Types.h"
#include "../utils/Constants.h"

extern int minShiftBishop[BOARD_SIZE];

/**
 * Returns a bitboard which contains the bishop valid moves if the board was empty.
 * Note that we do not return the last square that the bishop can move.
 * We can use a trick to account for that, and this will reduce the number of total masks
*/
u64 bishopMovementMaskFromPosition(int position);

void fillBishopBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalMoves);

#endif /* BFF9D0FF_89E9_404D_BAB7_475DDDC1578B */
