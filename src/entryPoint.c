#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "UCICommandProcessing.h"
#include "magicBitBoard/MagicBitBoard.h"
#include "state/ZobristKey.h"
#include "chessBot/PieceSquareTable.h"
#include "chessBot/TranspositionTable.h"

#define VERSION ("1.0")
#define AUTHOR ("Etienne Pinard")

static void initEngine() {
    if (!MagicBitBoard_init() ||
        !ZobristKey_init() ||
        !PieceSquareTable_init() ||
        !TTable_init() ||
        !Game_setupChesGame(&chessgame, &chessgame.currentPosition, INITIAL_FEN, 0, 0)) {
        printf("Failed to initialize the engine properly, terminating the program...\n");
        exit(EXIT_FAILURE);
    }

    printf("Engine version %s by %s is initialized and ready to go!\n", VERSION, AUTHOR);
}

static void terminateEngine() {
    MagicBitBoard_terminate();
    TTable_terminate();
}

#define STARTING_BUFFER_SIZE (128)

/**
 * @brief Reads a line of arbitrary length from stdin
 * 
 * @return char* A heap allocated char containing the line. It is the caller responsability to free this memory.
 */
static char *readArbitraryLongLineFromStdin() {
    char *buffer = calloc(STARTING_BUFFER_SIZE, sizeof(char));
    assert(buffer != NULL && "Buffer is null, Buy more RAM LOL");
    int numByteRead = 0;
    int capacity = STARTING_BUFFER_SIZE;

    while (true) {
        int byteRead = fgetc(stdin);
        if (byteRead == '\n' || byteRead == EOF) { break; }
        buffer[numByteRead] = (char) byteRead;        
        numByteRead++;

        if (numByteRead >= capacity) {
            capacity *= 2;
            buffer = realloc(buffer, capacity);
            assert(buffer != NULL && "Buffer is null, Buy more RAM LOL");
        }
    }
    buffer[numByteRead] = '\0';
    return buffer;
}

static void readUCICommands() {
    while (true) {
        char *command = readArbitraryLongLineFromStdin();
        if (!processUCICommand(command)) { break; }
        free(command);
    }
}

int main(void) {
    initEngine();
    readUCICommands();
    terminateEngine();
    return 0;
}