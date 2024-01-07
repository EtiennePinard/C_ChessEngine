#include <stdio.h>

#include "../src/state/GameState.h"
#include "../src/utils/FenString.h"
#include "../src/MoveGenerator.h"
#include "../src/ChessGameEmulator.h"
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "LogChessStructs.h"

char* startingFenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/* 
gcc 
    testing/testingBoard.c testing/logChessStructs.c 
    src/moveGenerator.c src/chessGameEmulator.c 
    src/utils/fenString.c src/utils/utils.c 
    src/state/gameState.c src/state/board.c src/state/piece.c src/state/move.c 
    src/magicBitBoard/magicBitBoard.c src/magicBitBoard/rook.c src/magicBitBoard/bishop.c 
-g -o testBoard 

gcc testing/testingBoard.c testing/logChessStructs.c src/moveGenerator.c src/chessGameEmulator.c src/utils/fenString.c src/utils/utils.c src/state/gameState.c src/state/board.c src/state/piece.c src/state/move.c src/magicBitBoard/magicBitBoard.c src/magicBitBoard/rook.c src/magicBitBoard/bishop.c -g -o testBoard 
*/
int main(int argc, char const *argv[]) {
    magicBitBoardInitialize();
    GameState startingState = { 0 }; 
    setGameStateFromFenString(
        "K7/8/8/4R3/8/8/8/k7 w - - 0 1",
        &startingState);
    printBoard(startingState.board);

    GameState previousStates[1] = { 0 };
    Move moves[MAX_LEGAL_MOVES + 1] = { [0 ... (MAX_LEGAL_MOVES)] = 0 };
    getValidMoves(moves, startingState, previousStates);
    printf("Number of moves: %d\n", nbMovesInArray(moves));
    magicBitBoardTerminate();
    // int index = 0;
    // while (1) {
    //     Move move = moves[index];
    //     index++;
    //     if (!move) {
    //         break;
    //     }
    //     printf("\nMaking ");
    //     printMove(move);
    //     GameState oldState = startingState;
    //     makeMove(move, &oldState);
    //     printBoard(oldState.board);
    // }
    return 0;
}
