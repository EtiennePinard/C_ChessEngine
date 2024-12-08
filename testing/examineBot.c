#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../src/utils/Utils.h"
#include "../src/chessBot/ChessBot.h"
#include "../src/chessBot/PieceSquareTable.h"
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/state/ZobristKey.h"

#include "../stockfishUCI/StockfishAPI.h"

#include "LogChessStructs.h"

#define BUF_SIZE 65536
// Number taken from https://chess.stackexchange.com/a/30006
// The post states an answer of 87, but's let's add 3 more characters just to be sure
#define LARGEST_FEN 90

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

void randomFenFromFile(FILE* file, u64 numLines, char* fenString) {
    u64 targetLine = random_u64() % (numLines + 1);

    char line[LARGEST_FEN];
    u64 currentLine = 0;
    while (currentLine < targetLine && fgets(line, sizeof(line), file) != NULL) {
        currentLine++;
    }

    strcpy(fenString, line);
}

#define KEY_Q 113

// TODO: When this is called the previousState explodes to a number bigger than 512, which is not supposed to happen
int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Usage: %s <fen strings file path> <stockfish executable path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* fenStringsPath = argv[1];
    char* stockfishPath = argv[2];

    FILE* positions = fopen(fenStringsPath, "r");
    if (positions == NULL) {
        printf("Unable to open the positions file\n");
        exit(EXIT_FAILURE);
    }
    
    magicBitBoardInitialize();
    zobristKeyInitialize();
    pieceSquareTableInitialize();

    u64 lineSize = countLinesInFile(positions);
    
    if (!stockfishInit(stockfishPath)) {
        printf("Error while initializing stockfish\n");
        exit(EXIT_FAILURE);
    }

    char fen[LARGEST_FEN] = { 0 };
    ChessGame game = { 0 };
    ChessPosition currentPosition = { 0 };
    while (true) {
        rewind(positions);
        randomFenFromFile(positions, lineSize, fen);
        setupChesGame(&game, &currentPosition, fen, (u32) 1, (u32) 1); // We don't care about time controls
        
        provideGameStateForBot(&game);
        setStockfishPosition(fen);

        int score = staticEvaluation();
        Move bestMove = think();

        float stockfishScore = stockfishStaticEvaluation();
        Move stockfishMove = stockfishBestMove(2); // Start with depth 2 for now

        printf("\n");
        printPosition(game.currentPosition, score, bestMove, stockfishScore, stockfishMove); 

        printf("Enter q to quit (Any key to continue): ");
        int c = getc(stdin);
        if (c == KEY_Q) {
            break;
        }
    }

    magicBitBoardTerminate();
    fclose(positions);
    return 0;
}


