#ifndef C5445595_E964_49BF_A12D_178666EDA063
#define C5445595_E964_49BF_A12D_178666EDA063

#include <stdint.h>

#define u64 uint64_t 
#define MAX_BLOCKING_BITBOARD_INCLUDING_OUTER_SQUARE (1 << 14)
#define MAX_BLOCKING_BITBOARD_NO_LAST_SQUARE (1 << 12)
#define BOARD_SIZE 64

u64 random_u64();
u64 random_u64_fewbits();

int trailingZeros_64(const u64 x);
int nbBitsSet(u64 num);

/**
 * Returns the number of pseudo legal moves bit board for a given position
*/
int nbOfPseudoLegalMoveBitBoardForPosition(int position);

u64 generateBlockingBitBoardFromIndex(int position, int index, int nbValidSquareForPiece, u64 movementMask);

#endif /* C5445595_E964_49BF_A12D_178666EDA063 */
