#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "magicBitBoard/MagicBitBoard.h"
#include "state/ZobristKey.h"
#include "state/EngineState.h"
#include "bot/PieceSquareTable.h"
#include "bot/TranspositionTable.h"
#include "utils/FenString.h"

#include "UCICommandProcessing.h"

static void initEngine() {
    if (!MagicBitBoard_init() ||
        !ZobristKey_init() ||
        !PieceSquareTable_init() ||
        !TranspositionTable_init() ||
        !FenString_setChessPositionFromCopiedFenString(INITIAL_FEN, &ourCurrentPosition)) {
        
        UCI_sendResponse("Failed to initialize the engine properly, terminating the program...\n");
        exit(EXIT_FAILURE);
    }

    UCI_sendResponse("%s version %s by %s is initialized and ready to go!\n", ENGINE_NAME, VERSION, AUTHOR);
}

static void terminateEngine() {
    MagicBitBoard_terminate();
    TranspositionTable_terminate();
    UCI_terminate();
}

#define STARTING_BUFFER_SIZE (128)

/**
 * @brief Reads a line of arbitrary length from stdin
 * 
 * @return char* A heap allocated char containing the line. It is the caller responsibility to free this memory.
 */
// TODO: Potential speedup, tokenize the command here instead of within the processUCICommand function
static char *readArbitraryLongLineFromStdin(char *buffer, int capacity) {
    assert(buffer != NULL && "Buffer is null, Buy more RAM LOL");
    int numByteRead = 0;

    while (true) {
        int byteRead = fgetc(stdin);
        if (byteRead == '\n' || byteRead == EOF) { break; }

        buffer[numByteRead++] = (char) byteRead;

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
    char *command = calloc(STARTING_BUFFER_SIZE, sizeof(char));
    while (true) {
        command = readArbitraryLongLineFromStdin(command, STARTING_BUFFER_SIZE);
        if (!UCI_processUCICommand(command)) { break; }
    }
    free(command);
}

int main(void) {
    initEngine();
    readUCICommands();
    terminateEngine();
    return 0;
}