#include <stdio.h>
#include <string.h>

#include "UCICommandProcessing.h"
#include "magicBitBoard/MagicBitBoard.h"
#include "state/ZobristKey.h"
#include "chessBot/PieceSquareTable.h"

#define VERSION ("1.0")
#define AUTHOR ("Etienne Pinard")

void initEngine() {
    magicBitBoardInitialize();
    zobristKeyInitialize();
    pieceSquareTableInitialize();
    setupChesGame(&chessgame, &chessgame.currentPosition, INITIAL_FEN, 3 * 60 * 1000, 3 * 60 * 1000);

    printf("Engine version %s by %s is initialized and ready to go!\n", VERSION, AUTHOR);
}

void terminateEngine() {
    magicBitBoardTerminate();
}

void readUCICommands() {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        // fgets always puts the new line in buffer so we remove it
        buffer[strcspn(buffer, "\n")] = '\0';

        if (!processUCICommand(buffer)) { break; }
    }
}

int main() {
    initEngine();
    readUCICommands();
    terminateEngine();
    return 0;
}