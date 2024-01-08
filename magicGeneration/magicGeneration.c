#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "Rook.h"
#include "Bishop.h"
#include "Utils.h"
#include "Logging.h"

typedef struct MagicData {
    u64 magic;
    int shift;
} MagicData;

/** 
 * These values were taken from the table in https://www.chessprogramming.org/Best_Magics_so_far
 * It only includes magics that have shifts which are bigger than the number of bits set in the movement mask
*/
MagicData bestRookMagicData[BOARD_SIZE] = {
    [0 ... 47] = (MagicData) { 0 },
    [48] = { .magic = 0x48FFFE99FECFAA00UL, .shift = 54 },
    [49] = { .magic = 0x48FFFE99FECFAA00UL, .shift = 55 },
    [50] = { .magic = 0x497FFFADFF9C2E00UL, .shift = 55 },
    [51] = { .magic = 0x613FFFDDFFCE9200UL, .shift = 55 },
    [52] = { .magic = 0xffffffe9ffe7ce00UL, .shift = 55 },
    [53] = { .magic = 0xfffffff5fff3e600UL, .shift = 55 },
    [54] = { .magic = 0x0003ff95e5e6a4c0UL, .shift = 55 },
    [55] = { .magic = 0x510FFFF5F63C96A0UL, .shift = 54 },
    [56] = { .magic = 0xEBFFFFB9FF9FC526UL, .shift = 53 },
    [57] = { .magic = 0x61FFFEDDFEEDAEAEUL, .shift = 54 },
    [58] = { .magic = 0x53BFFFEDFFDEB1A2UL, .shift = 54 },
    [59] = { .magic = 0x127FFFB9FFDFB5F6UL, .shift = 54 },
    [60] = { .magic = 0x411FFFDDFFDBF4D6UL, .shift = 54 },
    [61] = (MagicData) { 0 }, // There is no better magic found for square 61
    [62] = { .magic = 0x0003ffef27eebe74UL, .shift = 54 },
    [63] = { .magic = 0x7645FFFECBFEA79EUL, .shift = 53}
};

/** 
 * These values were taken from the table in https://www.chessprogramming.org/Best_Magics_so_far
 * It only includes magics that have shifts which are bigger than the number of bits set in the movement mask
*/
MagicData bestBishopMagicData[BOARD_SIZE] = {
    [0 ] = { .magic = 0xffedf9fd7cfcffffUL, .shift = 59 },
    [1 ] = { .magic = 0xfc0962854a77f576UL, .shift = 60 },
    [2 ... 5] = (MagicData) { 0 },
    [6 ] = { .magic = 0xfc0a66c64a7ef576UL, .shift = 60 },
    [7 ] = { .magic = 0x7ffdfdfcbd79ffffUL, .shift = 59 },
    [8 ] = { .magic = 0xfc0846a64a34fff6UL, .shift = 60 },
    [9 ] = { .magic = 0xfc087a874a3cf7f6UL, .shift = 60 },
    [10 ... 13] = (MagicData) { 0 },
    [14] = { .magic = 0xfc0864ae59b4ff76UL, .shift = 60 },
    [15] = { .magic = 0x3c0860af4b35ff76UL, .shift = 60 },
    [16] = { .magic = 0x73C01AF56CF4CFFBUL, .shift = 60 },
    [17] = { .magic = 0x41A01CFAD64AAFFCUL, .shift = 60 },
    [18 ... 21] = (MagicData) { 0 },
    [22] = { .magic = 0x7c0c028f5b34ff76UL, .shift = 60 },
    [23] = { .magic = 0xfc0a028e5ab4df76UL, .shift = 60 },
    [24 ... 39] = (MagicData) { 0 },
    [40] = { .magic = 0xDCEFD9B54BFCC09FUL, .shift = 60 },
    [41] = { .magic = 0xF95FFA765AFD602BUL, .shift = 60 },
    [42 ... 45] = (MagicData) { 0 },
    [46] = { .magic = 0x43ff9a5cf4ca0c01UL, .shift = 60 },
    [47] = { .magic = 0x4BFFCD8E7C587601UL, .shift = 60 },
    [48] = { .magic = 0xfc0ff2865334f576UL, .shift = 60 },
    [49] = { .magic = 0xfc0bf6ce5924f576UL, .shift = 60 },
    [50 ... 53] = (MagicData) { 0 },
    [54] = { .magic = 0xc3ffb7dc36ca8c89UL, .shift = 60 },
    [55] = { .magic = 0xc3ff8a54f4ca2c89UL, .shift = 60 },
    [56] = { .magic = 0xfffffcfcfd79edffUL, .shift = 59 },
    [57] = { .magic = 0xfc0863fccb147576UL, .shift = 60 },
    [58 ... 61] = (MagicData) { 0 },
    [62] = { .magic = 0xfc087e8e4bb2f736UL, .shift = 60 },
    [63] = { .magic = 0x43ff9e4ef4ca2c89UL, .shift = 59 },
};
/*
    [] = { .magic = UL, .shift = },
*/
typedef enum PieceOptions {
    ROOK,
    BISHOP
} PieceOptions; 

typedef struct FindingMagicBitBoard {
    PieceOptions piece;
    int position;

    u64 currentMagicGuess;
    int shift;

    u64 blockingBitBoards[MAX_BLOCKING_BITBOARD_NO_LAST_SQUARE];
    u64 blockingBitBoardToPseudoLegalmoves[MAX_BLOCKING_BITBOARD_NO_LAST_SQUARE];
} FindingMagicBitBoard;

typedef struct MagicResult {
    PieceOptions resultForPiece;
    int resultForPosition;

    MagicData data;

} MagicResult;

bool doesShiftValueLeadToCollisions(int position, int shiftToUse, int nbValidSquareForPosition, u64 magic, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves, u64* used) {
    int nbBlockingBitBoard = 1 << nbValidSquareForPosition;
    bool result = false;

    for (int i = 0; i < nbBlockingBitBoard; i++) {
        u64 blockingBitBoard = blockingBitBoards[i];
        u64 correctMoveBitBoard = blockingBitBoardToPseudoLegalmoves[i];
        
        u64 key = (blockingBitBoard * magic) >> shiftToUse;

        if (used[key] != 0ULL && used[key] != correctMoveBitBoard) {
            // This magic has a collision which does not lead to the correct moves bit board
            // This magic is thus invalid
            result = true;
            break;
        } 
        // We did not find a collision for this key
        used[key] = correctMoveBitBoard;
    }
    return result;
}

void prepareFindingMagicBitBoardStruct(FindingMagicBitBoard* data) {
    if (data->piece == ROOK) {
        data->shift = 64 - minShiftRook[data->position];
        fillRookBlockingBitBoardAndPseudoLegalMoveArray(data->position, data->blockingBitBoards, data->blockingBitBoardToPseudoLegalmoves);
    } else {
        data->shift = 64 - minShiftBishop[data->position];
        fillBishopBlockingBitBoardAndPseudoLegalMoveArray(data->position, data->blockingBitBoards, data->blockingBitBoardToPseudoLegalmoves); 
    }
}

void bruteForceMagic(FindingMagicBitBoard* data) {
    srand(time(NULL));

    u64 movementMask = data->piece == ROOK ? rookMovementMaskFromPosition(data->position) : bishopMovementMaskFromPosition(data->position);
    prepareFindingMagicBitBoardStruct(data);

    size_t size = 1 << (64 - data->shift);
    u64 used[size];
    do {
        data->currentMagicGuess = random_u64_fewbits();
        // If there is not enough significant bits set before shifting, the magic will probably have collisions
        if (nbBitsSet((movementMask * data->currentMagicGuess) & 0xFF00000000000000ULL) < 6) { continue; }

        memset(used, 0, sizeof(u64) * size);

    } while (doesShiftValueLeadToCollisions(data->position, data->shift, 64 - data->shift, data->currentMagicGuess, data->blockingBitBoards, data->blockingBitBoardToPseudoLegalmoves, used));
}

void verifyMagic(FindingMagicBitBoard data) {
    size_t size = 1 << (64 - data.shift);
    u64 used[size];
    memset(used, 0, sizeof(u64) * size);
    if (!doesShiftValueLeadToCollisions(data.position, data.shift, 64 - data.shift, data.currentMagicGuess, data.blockingBitBoards, data.blockingBitBoardToPseudoLegalmoves, used)) {
        printf("The magic and shift value are correct!\n");
    } else {
        printf("The magic and shift value are not correct :(\n");
    }
}

MagicResult singleThreadApproach(int position, PieceOptions piece) {
    FindingMagicBitBoard data = { 0 };
    data.piece = piece;
    data.position = position;
    
    // printf("Generating magic and shifts for position %d and shift %d\n", data.position, data.shift);

    bruteForceMagic(&data);

    MagicResult result = {
        .resultForPiece = data.piece,
        .resultForPosition = data.position,
        .data.magic = data.currentMagicGuess,
        .data.shift = data.shift
    };

    return result;
}

void verifyBestMagicValues() {
    FindingMagicBitBoard data = { 0 };
    size_t maxSize = (1 << 12);
    u64 used[maxSize];
    
    printf("Verify rook's magic\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        MagicData rook = bestRookMagicData[i];
        if (rook.magic == 0UL) { continue; }
        data.piece = ROOK;
        data.position = i;
        prepareFindingMagicBitBoardStruct(&data);
        data.currentMagicGuess = rook.magic;
        data.shift = rook.shift;
        size_t size = 1 << (64 - data.shift);
        memset(used, 0, sizeof(u64) * size);
        if (doesShiftValueLeadToCollisions(data.position, data.shift, 64 - data.shift, data.currentMagicGuess, data.blockingBitBoards, data.blockingBitBoardToPseudoLegalmoves, used)) {
            printf("\tERROR square %d, magic: %lu, shift: %d\n", i, data.currentMagicGuess, data.shift);
        } else {
            printf("\tPosition %d magic and shift are correct!\n", i);
        }
    }

    printf("Verify bishop's magic\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        MagicData bishop = bestBishopMagicData[i];
        if (bishop.magic == 0UL) { continue; }
        data.piece = BISHOP;
        data.position = i;
        prepareFindingMagicBitBoardStruct(&data);
        data.currentMagicGuess = bishop.magic;
        data.shift = bishop.shift;
        size_t size = 1 << (64 - data.shift);
        memset(used, 0, sizeof(u64) * size);
        if (doesShiftValueLeadToCollisions(data.position, data.shift, 64 - data.shift, data.currentMagicGuess, data.blockingBitBoards, data.blockingBitBoardToPseudoLegalmoves, used)) {
            printf("\tERROR square %d, magic: %lu, shift: %d\n", i, data.currentMagicGuess, data.shift);
        } else {
            printf("\tPosition %d magic and shift are correct!\n", i);
        }
    }
}

// IDEA: Make a neural network to guess shifts and magic value.
// The goal of the nn is to find a magic number which maximized the shifts value but still returns false if doesShiftValueLeadToCollisions() is called

// gcc -g -o generateMagic magicGeneration.c rook.c bishop.c logging.c utils.c && ./generateMagic
int main(int argc, char const *argv[]) {

    FILE* output = fopen("output.txt", "w");

    // Generating the rook/bishop masks array
    u64 movementMask; 

    // Rook movement mask array
    fprintf(output, "u64 rookMovementMask[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        movementMask = rookMovementMaskFromPosition(i);
        fprintf(output, "%luUL", movementMask);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }

    fprintf(output, "\n");

    // Bishops movement mask array
    fprintf(output, "u64 bishopMovementMask[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        movementMask = bishopMovementMaskFromPosition(i);
        fprintf(output, "%luUL", movementMask);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }

    fprintf(output, "\n");

    // Generating the rook/bishop magic/shift array and index offset
    u64 magics[BOARD_SIZE];
    int shifts[BOARD_SIZE];

    // Rook magics/shifts array
    for (int i = 0; i < BOARD_SIZE; i++) {
        MagicData rookData = bestRookMagicData[i];
        if (rookData.magic == 0UL) {
            MagicResult result = singleThreadApproach(i, ROOK);
            rookData = result.data;
        }
        magics[i] = rookData.magic;
        shifts[i] = rookData.shift;
    }

    // 3 loops cause ... yeah
    fprintf(output, "u64 rookMagics[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(output, "%luUL", magics[i]);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }
    fprintf(output, "int rookShifts[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(output, "%d", shifts[i]);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }
    fprintf(output, "int rookIndexOffset[BOARD_SIZE] = {");
    int indexOffset = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (i > 0) {
            indexOffset += 1 << (64 - shifts[i - 1]);
        }
        fprintf(output, "%d", indexOffset);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
            indexOffset += 1 << (64 - shifts[i]);
            fprintf(output, "#define ROOK_PSEUDO_LEGAL_MOVES_ARRAY_SIZE %d\n", indexOffset);
        }
    }

    fprintf(output, "\n");

    // Bishop magics/shifts array
    for (int i = 0; i < BOARD_SIZE; i++) {
        MagicData bishopData = bestBishopMagicData[i];
        if (bishopData.magic == 0UL) {
            MagicResult result = singleThreadApproach(i, BISHOP);
            bishopData = result.data;
        }
        magics[i] = bishopData.magic;
        shifts[i] = bishopData.shift;
    }

    // 3 loops cause ... yeah
    fprintf(output, "u64 bishopMagics[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(output, "%luUL", magics[i]);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }
    fprintf(output, "int bishopShifts[BOARD_SIZE] = {");
    for (int i = 0; i < BOARD_SIZE; i++) {
        fprintf(output, "%d", shifts[i]);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
        }
    }
    fprintf(output, "int bishopIndexOffset[BOARD_SIZE] = {");
    indexOffset = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (i > 0) {
            indexOffset += 1 << (64 - shifts[i - 1]);
        }
        fprintf(output, "%d", indexOffset);
        if (i + 1 != BOARD_SIZE) {
            fprintf(output, ", ");
        } else {
            fprintf(output, "};\n");
            indexOffset += 1 << (64 - shifts[i]);
            fprintf(output, "#define BISHOP_PSEUDO_LEGAL_MOVES_ARRAY_SIZE %d\n", indexOffset);
        }
    }

    fclose(output);

    return 0;
}