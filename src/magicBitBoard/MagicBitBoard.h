#include "../utils/Utils.h"

void magicBitBoardInitialize();
void magicBitBoardTerminate();

extern u64 rookMovementMask[BOARD_SIZE];
u64 getRookPseudoLegalMovesBitBoard(int position, u64 blockingBitBoard);

extern u64 bishopMovementMask[BOARD_SIZE];
u64 getBishopPseudoLegalMovesBitBoard(int position, u64 blockingBitBoard);

