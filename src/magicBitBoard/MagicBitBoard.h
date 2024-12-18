#include <stdbool.h>
#include "../utils/Utils.h"

/**
 * @brief Initializes the magic bit boards 
 * 
 * @return true If the magic bit boards were initialize properly
 * @return false If the magic bit boards could not be initialize properly
 */
bool MagicBitBoard_init();
void MagicBitBoard_terminate();

extern u64 kingMovementMask[BOARD_SIZE];

extern u64 knightMovementMask[BOARD_SIZE];

extern u64 rookMovementMask[BOARD_SIZE];
u64 MagicBitBoard_getRookPseudoLegalMovesBitBoard(int position, u64 blockingBitBoard);

extern u64 bishopMovementMask[BOARD_SIZE];
u64 MagicBitBoard_getBishopPseudoLegalMovesBitBoard(int position, u64 blockingBitBoard);

