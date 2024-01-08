#ifndef BF86F4AA_8799_422F_9A8A_0C8312B52306
#define BF86F4AA_8799_422F_9A8A_0C8312B52306

#include "Utils.h"

/**
 * Returns a bitboard which contains the rooks valid moves if the board was empty.
 * Note that we do not return the last square that the rook can move.
 * We can use a trick to account for that, and this will reduce the number of total masks
*/
u64 rookMovementMaskFromPosition(int position);

u64 rookPseudoLegalMovesBitBoardFromBlockingBitBoard(int position, u64 blockingBitBoard);

void fillRookBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves);

extern int minShiftRook[BOARD_SIZE];

#endif /* BF86F4AA_8799_422F_9A8A_0C8312B52306 */
