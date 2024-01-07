#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/MoveGenerator.h"
#include "../src/utils/FenString.h"
#include "../src/ChessGameEmulator.h"
#include "LogChessStructs.h"

#define TEST_ITERATION 1

int maximumDepth;

u64 perft(int depth, GameState* achievedStates) {
  if (depth == 0) { return 1; }
  int nbMoveMade = maximumDepth - depth;
  GameState previousState = achievedStates[nbMoveMade];
  GameState previousStates[1] = { 0 };
  
  Move moves[MAX_LEGAL_MOVES + 1] = { [0 ... (MAX_LEGAL_MOVES)] = 0 };

  getValidMoves(moves, previousState, previousStates); // We do not care about draw by repetition
  int nbOfMoves, i;
  u64 nodes = 0;
  nbOfMoves = nbMovesInArray(moves);
  if (nbOfMoves == 1) {
    int flag = flagFromMove(moves[0]);
    if (flag == DRAW || flag == STALEMATE || flag == CHECKMATE) {
      // Game is finished, continuing to next move
      return 0;
    }
  }

  if (depth == 1) {
    return nbOfMoves;
  }

  for (i = 0; i < nbOfMoves; i++) {

    GameState newState = previousState;
    Move move = moves[i];
    makeMove(move, &newState); // Move is made
    
    if (depth != 1) { // when depth == 1 it would write to invalid indices
      achievedStates[nbMoveMade + 1] = newState;
    }

    u64 moveOutput = perft(depth - 1, achievedStates); // We generate the moves for the next perft
    // if (depth == maximumDepth) {
    //   printMoveToAlgebraic(move);
    //   printf(": %lu\n", moveOutput);
    // }
    nodes += moveOutput;
  }
  
  return nodes;
}

char* startingFenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// To compile and run the program: ./perft <depth>
// To check for memory leaks that program: valgrind --leak-check=full --track-origins=yes -s ./perftTesting <depth>
int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Usage: ./%s <depth>\n", argv[0]);
    return 0;
  }

  maximumDepth = atoi(argv[1]);

  if (maximumDepth <= 0) {
    printf("Invalid depth %d\n", maximumDepth);
    return 0;
  }

  magicBitBoardInitialize();

  GameState* achievedStates = malloc(sizeof(GameState) * maximumDepth);
  
  GameState startingState = { 0 }; 
  
  setGameStateFromFenString(
    startingFenString, &startingState);

  achievedStates[0] = startingState;

  double averageExecutionTime = 0;
  clock_t begin, end;
  u64 perftResult;

  for (int iterations = 0; iterations < TEST_ITERATION; iterations++) {
    begin = clock();
    perftResult = perft(maximumDepth, achievedStates);
    end = clock();
    double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
    averageExecutionTime += timeSpent;
    printf("ITERATION #%d, Time: %fs, Perft: %lu\n", iterations, timeSpent, perftResult);
  }
  averageExecutionTime /= TEST_ITERATION;
  printf("Perft depth %d took on average %fms (%fs)\n", maximumDepth, averageExecutionTime * 1000, averageExecutionTime);
  free(achievedStates);
  magicBitBoardTerminate();
  return 0;
}

// Peft 5 of initial board with my engine: 4 866 385
// Peft 5 of initial board stockfish:      4 865 609 

// 4 085 603 Engine
// 4 085 603 Actual
// I had to remove some memory leaks, else the kernel would kill the program (it was leaking too much memory...)


// Perft 5: 193 690 690 (Engine)
// Perft 5: 193 690 690 (Actual)
// I am so happy to have finally done it

// On February 26th 2023, I completed the position 3 table from this page: https://www.chessprogramming.org/Perft_Results#Position_3
// Perft 8: 3 009 794 393 (engine)
// Perft 8: 3 009 794 393 (table)

