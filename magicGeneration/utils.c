#include <stdlib.h>
#include <stdio.h>
#include "Utils.h"

u64 random_u64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

u64 random_u64_fewbits() {
  return random_u64() & random_u64() & random_u64();
}

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
int trailingZeros_64(const u64 x) {
    return __builtin_ctzl(x);
}

int nbBitsSet(u64 num) {
  int result;
  while(num) {
    result++;
    num &= num - 1;
  }
  return result;
}

u64 generateBlockingBitBoardFromIndex(int position, int index, int nbValidSquareForPiece, u64 movementMask) {
    if (index >= (1 << nbValidSquareForPiece)) {
        printf("ERROR: index %d is invalid in function `generateBlockingBitBoardFromPosition`\n", index);
    }
    u64 result = (u64) 0;
    for (int i = 0; i < nbValidSquareForPiece; i++) {
        u64 bitToShift = (u64) ((index >> i) & 1);
        int movementMaskNextBit = trailingZeros_64(movementMask);
        result |= bitToShift << movementMaskNextBit;
        movementMask &= movementMask - 1;
    }
    return result;
}

int nbOfPseudoLegalMoveBitBoardForPosition(int position) {
    int x = position % 8;
    int y = position / 8;
    
    int factor1 = x;
    factor1 = factor1 == 0 ? 1 : factor1;
    int factor2 = 7 - x;
    factor2 = factor2 == 0 ? 1 : factor2;

    int factor3 = y;
    factor3 = factor3 == 0 ? 1 : factor3;

    int factor4 = 7 - y;
    factor4 = factor4 == 0 ? 1 : factor4;

    return factor1 * factor2 * factor3 * factor4;
}