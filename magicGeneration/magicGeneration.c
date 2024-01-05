#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "Utils.h"
#include "Logging.h"

// ACRONYM: PLMBB -> Pseudo Legal Move BitBoard

// ChatGPT solution lol!
volatile int exitFlag = 0;

typedef struct FindingMagicBitBoard {
    int position;

    u64 currentMagicGuess;
    int shift;

    u64 blockingBitBoards[MAX_BLOCKING_BITBOARD_NO_LAST_SQUARE];
    u64 blockingBitBoardToPseudoLegalmoves[MAX_BLOCKING_BITBOARD_NO_LAST_SQUARE];
} FindingMagicBitBoard;

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

/**
 * Returns the number of pseudo legal moves bit board for a given position
*/
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

/**
 * Returns a bitboard which contains the rooks valid moves if the board was empty.
 * Note that we do not return the last square that the rook can move.
 * We can use a trick to account for that, and this will reduce the number of total masks
*/
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

static inline u64 generateBlockingBitBoardFromIndex(int position, int index) {
    // We assume the index is valid, so between 0 and 2^14 - 1
    int nbValidSquareForRook = minShiftRook[position];
    if (index >= (1 << nbValidSquareForRook)) {
        printf("ERROR: index %d is invalid in function `generateBlockingBitBoardFromPosition`\n", index);
    }
    u64 result = (u64) 0;
    u64 movementMask = rookMovementMaskFromPosition(position);
    for (int i = 0; i < nbValidSquareForRook; i++) {
        u64 bitToShift = (u64) ((index >> i) & 1);
        int movementMaskNextBit = trailingZeros_64(movementMask);
        result |= bitToShift << movementMaskNextBit;
        movementMask &= movementMask - 1;
    }
    return result;
}

u64 pseudoLegalMovesBitBoardFromBlockingBitBoard(int position, u64 blockingBitBoard) {
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

void fillBlockingBitBoardAndPseudoLegalMoveArray(int position, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves) {
    int nbBlockingBitBoard = 1 << minShiftRook[position];
    for (int i = 0; i < nbBlockingBitBoard; i++) {
        blockingBitBoards[i] = generateBlockingBitBoardFromIndex(position, i);
        blockingBitBoardToPseudoLegalmoves[i] = pseudoLegalMovesBitBoardFromBlockingBitBoard(position, blockingBitBoards[i]);;
    }
}

/**
 * Fills the pseudo legal moves array
*/
bool fillPseudoLegalMovesArray(int position, int shiftToUse, u64 magic, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves, u64* rookPseudoLegalMovesBitBoard) {
    size_t size = ((size_t) (1)) << (64 - shiftToUse);
    rookPseudoLegalMovesBitBoard = calloc(size, sizeof(u64));
    int nbBlockingBitBoard = 1 << minShiftRook[position];
    for (int i = 0; i < nbBlockingBitBoard; i++) {
        u64 blockingBitBoard = blockingBitBoards[i];
        u64 correctMoveBitBoard = blockingBitBoardToPseudoLegalmoves[i];
        
        u64 key = (blockingBitBoard * magic) >> shiftToUse;
        if (rookPseudoLegalMovesBitBoard[key] == 0 || rookPseudoLegalMovesBitBoard[key] == correctMoveBitBoard) {
            rookPseudoLegalMovesBitBoard[key] = correctMoveBitBoard;
        } else {
            free(rookPseudoLegalMovesBitBoard);
            return false;
        }
    }
    free(rookPseudoLegalMovesBitBoard);
    return true;
}

bool doesShiftValueLeadToCollisions(int position, int shiftToUse, u64 magic, u64* blockingBitBoards, u64* blockingBitBoardToPseudoLegalmoves, u64* used) {
    int nbBlockingBitBoard = 1 << minShiftRook[position];
    bool result = false;

    for (int i = 0; i < nbBlockingBitBoard; i++) {
        u64 blockingBitBoard = blockingBitBoards[i];
        u64 correctMoveBitBoard = blockingBitBoardToPseudoLegalmoves[i];
        
        u64 key = (blockingBitBoard * magic) >> shiftToUse;

        // used[key] != 0 && used[key] != correctMovesBitBoard
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

void bruteForceRookMagic(FindingMagicBitBoard* data) {
    srand(time(NULL));
    fillBlockingBitBoardAndPseudoLegalMoveArray(data->position, data->blockingBitBoards, data->blockingBitBoardToPseudoLegalmoves);
    u64 movementMask = rookMovementMaskFromPosition(data->position);
    size_t size = 1 << (64 - data->shift);
    u64 used[size];
    do {
        data->currentMagicGuess = random_u64_fewbits();
        // If there is not enough significant bits set before shifting, the magic will probably have collisions
        if (nbBitsSet((movementMask * data->currentMagicGuess) & 0xFF00000000000000ULL) < 6) { continue; }

        memset(used, 0, sizeof(u64) * size);

    } while (doesShiftValueLeadToCollisions(data->position, data->shift, data->currentMagicGuess, data->blockingBitBoards, data->blockingBitBoardToPseudoLegalmoves, used));
}

void *bruteForceRookMagicThread(void* arg) {
    bruteForceRookMagic((FindingMagicBitBoard*) arg);
    exitFlag = 1;
    pthread_exit(NULL);
}

void verifyMagic(FindingMagicBitBoard data) {
    size_t size = 1 << (64 - data.shift);
    u64 used[size];
    memset(used, 0, sizeof(u64) * size);
    if (!doesShiftValueLeadToCollisions(data.position, data.shift, data.currentMagicGuess, data.blockingBitBoards, data.blockingBitBoardToPseudoLegalmoves, used)) {
        printf("The magic and shift value are correct!\n");
    } else {
        printf("The magic and shift value are not correct :(\n");
    }
}

void singleThreadApproach(int position) {
    FindingMagicBitBoard data = { 0 };
    int shift = 64 - minShiftRook[position];
    data.position = position;
    data.shift = shift;
    
    printf("Generating magic and shifts for position %d and shift %d\n", data.position, data.shift);

    bruteForceRookMagic(&data);

    logMagic(data.currentMagicGuess, data.shift);
    verifyMagic(data);
}

// gcc -g -o test magicGeneration.c logging.c utils.c -lpthread && ./test 
void multipleThreadingApproach(int position, int nbThreads) {
    pthread_t threads[nbThreads];
    FindingMagicBitBoard threadParams[nbThreads];

    int shift = 64 - minShiftRook[position];

    printf("Generating magic and shifts for position %d, shift %d, and %d threads\n", position, shift, nbThreads);

    // Create threads
    for (int i = 0; i < nbThreads; i++) {
        threadParams[i].position = position;
        threadParams[i].shift = shift; 
        printf("Starting thread %d\n", i);
        if (pthread_create(&threads[i], NULL, bruteForceRookMagicThread, (void*) &threadParams[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < nbThreads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            return;
        }
    }

    printf("We have found a suitable magic value!\n");

    for (int i = 0; i < nbThreads; i++) {
        FindingMagicBitBoard data = threadParams[i];
        if (data.shift == -1) {
            continue;
        }

        logMagic(data.currentMagicGuess, data.shift);
        verifyMagic(data);
    }

}

// IDEA: Make a neural network to guess shifts and magic value.
// The goal of the nn is to find a magic number which maximized the shifts value but still return true if verify() is called
int main(int argc, char const *argv[]) {

    int position = 0;

    // Magic number and shift value found by Sebastian Lague
    // magic = 468374916371625120;
    // shift = 52;

    singleThreadApproach(position);

    return 0;
}