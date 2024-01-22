#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
int trailingZeros_64(const u64 x) {
    return __builtin_ctzl(x);
}

u64 random_u64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

void printBitBoard(u64 bitboard) {
    for (int i = 0; i < 64; i++) {
        printf("%ld", (bitboard >> i) & 1);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
}

void printBin(const u64 num) {
    printf("0b");
    for (int i = 63; i >= 0; i--) {
        printf("%lu", (num >> i) & 1);
    }
    printf("\n");
}

