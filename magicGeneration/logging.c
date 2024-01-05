#include <stdio.h>
#include "Logging.h"
#include "Utils.h"

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

void logMagic(u64 magic, int shift) {
    printf("\tMagic: %lu\n", magic);
    printf("\t");
    printBin(magic);
    printf("\tShift: %d\n", shift);
}