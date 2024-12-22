#include "RepetitionTable.h"
#include <stdio.h>

#define REPETITION_TABLE_CAPACITY (1 << 9)

/*
    IMPORTANT! Notice that previousStates is initialized to 512.
    This means that we only support games which are 512 moves long.
    This could be like a huge flaw in the engine, since legal games
    can theoriticaly go to 6000 moves.

    However it is important to note that 6000 * 64 bit * 1kb / 1024 bit = 375kb,
    which is a bit too much memory to store on the stack for my taste.
    With 512 moves, we only store 32 kb, which is a lot, I guess, but it's fine by me

    Also like the average chess game is about 40 moves, so its not
    like were gonna run into this issue anytime soon (I really hope so)

    The reason I did this instead of a dynamic array is that this is faster 
    than a dynamic array and less error prone (if we don't run out of space)
 */
ZobristKey repetitionTable[REPETITION_TABLE_CAPACITY];
u16 currentIndex = 0;
u16 rootPosition = 0;

bool RepetitionTable_storeKey(ZobristKey key) {
    if (currentIndex == REPETITION_TABLE_CAPACITY) {
        printf("ERROR: The repetition table is out of space, this game is longer than %d moves!\n", REPETITION_TABLE_CAPACITY);
        return false;
    }
    repetitionTable[currentIndex++] = key;
    return true;
}

void RepetitionTable_setCurrentIndexAsRootPosition() { rootPosition = currentIndex; }

void RepetitionTable_returnToRootPosition() { currentIndex = rootPosition; }

bool RepetitionTable_isKeyContainedTwiceInTable(ZobristKey key) {
    bool hasOneDuplicate = false;
    bool result = false;
    for (u16 index = 0; index < currentIndex; index++) {
        if (key == repetitionTable[index]) {
            if (hasOneDuplicate) {
                result = true;
                break;
            }
            hasOneDuplicate = true;
        }
    }
    return result;
}