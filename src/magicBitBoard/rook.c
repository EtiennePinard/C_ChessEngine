#include "Rook.h"

int minShiftRook[BOARD_SIZE] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

u64 rookMovementMaskFromPosition(int position) {
    u64 result = (u64) 0;
    
    int x = position % 8;
    int y = position / 8;

    // 4 loops for 4 directions
    u64 mask = (u64) 1;    
    for (int i = 1; x + i <= 6; i++) {
        result |= mask << (position + i);
    }

    for (int i = 1; x - i >= 1; i++) {
        result |= mask << (position - i);
    }

    for (int i = 1; y + i <= 6; i++) {
        result |= mask << (position + i * 8);
    }

    for (int i = 1; y - i >= 1; i++) {
        result |= mask << (position - i * 8);
    }

    return result;
}

u64 rookPseudoLegalMovesBitBoardFromBlockingBitBoard(int position, u64 blockingBitBoard) {
    u64 result = (u64) 0;
    
    int x = position % 8;
    int y = position / 8;

    // 4 loops for 4 directions
    u64 mask = (u64) 1;
    for (int i = 1; x + i <= 7; i++) {
        result |= mask << (position + i);
        if ((blockingBitBoard >> (position + i)) & 1) {
            break;
        }
    }

    for (int i = 1; x - i >= 0; i++) {
        result |= mask << (position - i);
        if ((blockingBitBoard >> (position - i)) & 1) {
            break;
        }
    }

    for (int i = 1; y + i <= 7; i++) {
        result |= mask << (position + i * 8);
        if ((blockingBitBoard >> (position + i * 8)) & 1) {
            break;
        }
    }

    for (int i = 1; y - i >= 0; i++) {
        result |= mask << (position - i * 8);
        if ((blockingBitBoard >> (position - i * 8)) & 1) {
            break;
        }
    }

    return result;
}

static inline u64 generateBlockingBitBoardFromIndex(int index, int nbValidSquareForPiece, u64 movementMask) {
    u64 result = (u64) 0;
    for (int i = 0; i < nbValidSquareForPiece; i++) {
        u64 bitToShift = (u64) ((index >> i) & 1);
        int movementMaskNextBit = trailingZeros_64(movementMask);
        result |= bitToShift << movementMaskNextBit;
        movementMask &= movementMask - 1;
    }
    return result;
}

void fillRookBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves) {
    int nbValidSquareForRook = minShiftRook[position];
    u64 movementMask = rookMovementMaskFromPosition(position);
    int nbBlockingBitBoard = 1 << nbValidSquareForRook;
    for (int i = 0; i < nbBlockingBitBoard; i++) {
        blockingBitBoards[i] = generateBlockingBitBoardFromIndex(i, nbValidSquareForRook, movementMask);
        blockingBitBoardToPseudoLegalmoves[i] = rookPseudoLegalMovesBitBoardFromBlockingBitBoard(position, blockingBitBoards[i]);;
    }
}