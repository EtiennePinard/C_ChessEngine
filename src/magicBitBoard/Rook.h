#ifndef DCBCA724_033D_43CB_B729_61FB3A609CD3
#define DCBCA724_033D_43CB_B729_61FB3A609CD3

#include "../utils/Utils.h"

extern int minShiftRook[BOARD_SIZE];

/**
 * Returns a bitboard which contains the rooks valid moves if the board was empty.
 * Note that we do not return the last square that the rook can move.
 * We can use a trick to account for that, and this will reduce the number of total masks
*/
u64 rookMovementMaskFromPosition(int position);

void fillRookBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves);

#endif /* DCBCA724_033D_43CB_B729_61FB3A609CD3 */
