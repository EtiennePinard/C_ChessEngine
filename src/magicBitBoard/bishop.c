#include "Bishop.h"

int minShiftBishop[BOARD_SIZE] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

u64 bishopMovementMaskFromPosition(int position) {
    u64 result = (u64) 0;
    
    int ogX = position % 8;
    int ogY = position / 8;

    // 4 loops for 4 directions
    u64 mask = (u64) 1;    
    for (int x = ogX + 1, y = ogY + 1; x <= 6 && y <= 6; x++, y++) {
        result |= mask << (x + y * 8);
    }

    for (int x = ogX - 1, y = ogY + 1; x >= 1 && y <= 6; x--, y++) {
        result |= mask << (x + y * 8);
    }

    for (int x = ogX + 1, y = ogY - 1; x <= 6 && y >= 1; x++, y--) {
        result |= mask << (x + y * 8);
    }

    for (int x = ogX - 1, y = ogY - 1; x >= 1 && y >= 1; x--, y--) {
        result |= mask << (x + y * 8);
    }

    return result;
}

u64 bishopPseudoLegalMovesBitBoardFromBlockingBitBoard(int position, u64 blockingBitBoard) {
    u64 result = (u64) 0;
    
    int ogX = position % 8;
    int ogY = position / 8;

    // 4 loops for 4 directions
    u64 mask = (u64) 1;

    for (int x = ogX + 1, y = ogY + 1; x <= 6 && y <= 6; x++, y++) {
        result |= mask << (x + y * 8);
        if ((blockingBitBoard >> (x + y * 8)) & 1) {
            break;
        }
    }

    for (int x = ogX - 1, y = ogY + 1; x >= 1 && y <= 6; x--, y++) {
        result |= mask << (x + y * 8);
        if ((blockingBitBoard >> (x + y * 8)) & 1) {
            break;
        }
    }

    for (int x = ogX + 1, y = ogY - 1; x <= 6 && y >= 1; x++, y--) {
        result |= mask << (x + y * 8);
        if ((blockingBitBoard >> (x + y * 8)) & 1) {
            break;
        }
    }

    for (int x = ogX - 1, y = ogY - 1; x >= 1 && y >= 1; x--, y--) {
        result |= mask << (x + y * 8);
        if ((blockingBitBoard >> (x + y * 8)) & 1) {
            break;
        }
    }


    return result;
}

static inline u64 generateBlockingBitBoardFromIndex(int position, int index, int nbValidSquareForPiece, u64 movementMask) {
    u64 result = (u64) 0;
    for (int i = 0; i < nbValidSquareForPiece; i++) {
        u64 bitToShift = (u64) ((index >> i) & 1);
        int movementMaskNextBit = trailingZeros_64(movementMask);
        result |= bitToShift << movementMaskNextBit;
        movementMask &= movementMask - 1;
    }
    return result;
}

void fillBishopBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves) {
    int nbValidSquareForBishop = minShiftBishop[position];
    int nbBlockingBitBoard = 1 << nbValidSquareForBishop;
    u64 movementMask = bishopMovementMaskFromPosition(position);
    for (int i = 0; i < nbBlockingBitBoard; i++) {
        blockingBitBoards[i] = generateBlockingBitBoardFromIndex(position, i, nbValidSquareForBishop, movementMask);
        blockingBitBoardToPseudoLegalmoves[i] = bishopPseudoLegalMovesBitBoardFromBlockingBitBoard(position, blockingBitBoards[i]);;
    }
}