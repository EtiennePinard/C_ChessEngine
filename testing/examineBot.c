#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../src/utils/Utils.h"

#include "../src/chessBot/ChessBot.h"
#include "../src/chessBot/PieceSquareTable.h"

#include "../src/magicBitBoard/MagicBitBoard.h"

#include "../src/state/ZobristKey.h"

#include "LogChessStructs.h"

#define BUF_SIZE 65536

u64 countLinesInFile(FILE* file) {
    char buf[BUF_SIZE];
    u64 counter = 0;
    while (1) {
        size_t chunkSize = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file)) { return -1; }

        size_t i;
        for (i = 0; i < chunkSize; i++) {
            if (buf[i] == '\n') {
                counter++;
            }
        }

        if (feof(file)) {
            break;
        }
    }

    return counter;
}

char* randomFenFromFile(FILE* file, u64 numLines) {

    // Assuming a line is less than 256 bytes
    char result[256];

    u64 targetLine = random_u64() % (numLines + 1);

    char buf[BUF_SIZE];
    u64 counter = 0;

    int resultIndex = 0;
    bool hitEndOfLine = false;

    while (1) {
        size_t chunkSize = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file)) { return NULL; }

        size_t i;
        for (i = 0; i < chunkSize; i++) {
            if (buf[i] == '\n') {
                counter++;

                if (counter == targetLine) {
                    // Adding the line to the result
                    i++; // So that we don't hit the same '\n'
                    char bufferRead;
                    for (; i < chunkSize; resultIndex++, i++) {
                        bufferRead = buf[i];
                        if (bufferRead == '\n') {
                            result[resultIndex] = '\0';
                            hitEndOfLine = true;
                            break;
                        }
                        result[resultIndex] = bufferRead;
                    }
                    break;
                }
            }
        }

        if (counter == targetLine) {
            if (hitEndOfLine) { break; }
            
            char remainingLine[256];
            fgets(remainingLine, sizeof(char) * 256, file);

            for (i = 0; resultIndex < 256; i++, resultIndex++) {
                result[resultIndex] = remainingLine[i];
                if (remainingLine[i] == '\0') { break; }
            }
            break;
        }

        if (feof(file)) {
            break;
        }
    }

    char* resultPointer = malloc(sizeof(char) * resultIndex);
    memcpy(resultPointer, &result, resultIndex);

    return resultPointer;
}

// Compile and run: ./tBot
int main(void) {
    FILE* positions = fopen("./testing/chessPositions.txt", "r");
    if (positions == NULL) {
        printf("Unable to open the positions file\n");
        exit(EXIT_FAILURE);
    }
    
    magicBitBoardInitialize();
    zobristKeyInitialize();
    pieceSquareTableInitialize();

    u64 lineSize = countLinesInFile(positions);
    
    while (true) {
        rewind(positions);
        char* fen = randomFenFromFile(positions, lineSize);

        ChessGame* game = setupChesGame(NULL, fen);
        
        int score = staticEvaluation(game);
        Move bestMove = think(game);
        printPosition(*game->currentState, score, bestMove); 
    
        free(fen);
        freeChessGame(game);
        printf("Enter q to quit: ");
        int c = getc(stdin);
        if (c == 113) {
            break;
        }
    }

    magicBitBoardTerminate();
    zobristKeyTerminate();
    fclose(positions);
    return 0;
}


