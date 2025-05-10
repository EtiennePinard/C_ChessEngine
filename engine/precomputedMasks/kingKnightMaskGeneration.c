#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Utils.h"
#include "Logging.h"

bool isIndexValidForKnight(int currentIndex, int targetSquare, bool goDownOne) {
    int y = targetSquare / 8;
    bool wrappingCondition = (goDownOne) ? 
        abs(currentIndex / 8 - y) == 1 : 
        abs(currentIndex / 8 - y) == 2;
    return targetSquare >= 0 && targetSquare <= 63 && wrappingCondition;
}

u64 knightMovementMask(int currentIndex) {
    u64 result = (u64) 0;
    u64 toggle = (u64) 1;
    if (isIndexValidForKnight(currentIndex, currentIndex - 6 , true )) result ^= toggle << (currentIndex - 6 );
    if (isIndexValidForKnight(currentIndex, currentIndex - 15, false)) result ^= toggle << (currentIndex - 15);
    if (isIndexValidForKnight(currentIndex, currentIndex - 10, true )) result ^= toggle << (currentIndex - 10);
    if (isIndexValidForKnight(currentIndex, currentIndex - 17, false)) result ^= toggle << (currentIndex - 17);
    if (isIndexValidForKnight(currentIndex, currentIndex + 6 , true )) result ^= toggle << (currentIndex + 6 );
    if (isIndexValidForKnight(currentIndex, currentIndex + 10, true )) result ^= toggle << (currentIndex + 10);
    if (isIndexValidForKnight(currentIndex, currentIndex + 15, false)) result ^= toggle << (currentIndex + 15);
    if (isIndexValidForKnight(currentIndex, currentIndex + 17, false)) result ^= toggle << (currentIndex + 17);
    return result;
}

u64 kingMovementMask(int position) {
    u64 result = (u64) 0;
    u64 toggle = (u64) 1;
    if (position % 8 != 0 && position / 8 != 0) { result ^= toggle << (position - 9); }
    if (position / 8 != 0                     ) { result ^= toggle << (position - 8); }
    if (position % 8 != 7 && position / 8 != 0) { result ^= toggle << (position - 7); }
    if (position % 8 != 0                     ) { result ^= toggle << (position - 1); }
    if (position % 8 != 7                     ) { result ^= toggle << (position + 1); }
    if (position % 8 != 0 && position / 8 != 7) { result ^= toggle << (position + 7); }
    if (position / 8 != 7                     ) { result ^= toggle << (position + 8); }
    if (position % 8 != 7 && position / 8 != 7) { result ^= toggle << (position + 9); }
    return result;
}

// gcc -g -o generateMask kingKnightMaskGeneration.c logging.c utils.c && ./generateMask
int main(int argc, char const *argv[]) {
    FILE* output = fopen("precomputedMaskOutput.txt", "w");
    u64 movementMask;

    fprintf(output, "u64 kingMovementMask[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        
        movementMask = kingMovementMask(i);
        fprintf(output, "%luUL", movementMask);
        // printf("\nPos: %d, Mask: %lu\n", i, movementMask);
        // printBitBoard(movementMask);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }
    
    fprintf(output, "u64 knightMovementMask[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        
        movementMask = knightMovementMask(i);
        fprintf(output, "%luUL", movementMask);
        // printf("\nPos: %d, Mask: %lu\n", i, movementMask);
        // printBitBoard(movementMask);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }

    fclose(output);
    return 0;
}
