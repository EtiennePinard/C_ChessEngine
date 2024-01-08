#include "Utils.h"
#include <stdio.h>

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
int trailingZeros_64(const u64 x) {
    return __builtin_ctzl(x);
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

