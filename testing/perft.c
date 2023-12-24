#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "../src/moveGenerator.h"
#include "../src/fenString.h"
#include "../src/chessGameEmulator.h"
#include "logChessStructs.h"

#define TEST_ITERATION 100

typedef unsigned long long u64;

u64 perft(int depth, GameState* achievedStates, int maximumDepth, bool firstMoveOfMaxDepth) {
  if (depth == 0) { return 1; }
  int nbMoveMade = maximumDepth - depth;
  GameState previousState = achievedStates[nbMoveMade];
  GameStates previousStates = { 0 };
  
  Move moves[MAX_LEGAL_MOVES + 1] = { [0 ... (MAX_LEGAL_MOVES)] = 0 };

  getValidMoves(moves, previousState, previousStates); // We do not care about draw by repetition
  int nbOfMoves, i;
  u64 nodes = 0;
  nbOfMoves = nbMoves(moves);
  if (nbOfMoves == 1) {
    int flag = flag(moves[0]);
    if (flag == DRAW || flag == STALEMATE || flag == CHECKMATE) {
      // Game is finished, continuing to next move
      return 0;
    }
  }

  if (depth == 1) {
    return nbOfMoves;
  }

  for (i = 0; i < nbOfMoves; i++) {
  if (depth != 1) { // So that if we want to check moves at depth 1 with don't get memory leaks
      GameState newState = copyState(previousState);
      Move move = moves[i];
      makeMove(move, &newState); // Move is made

      if (i != 0 || !firstMoveOfMaxDepth) { // When it is the first time we have reached this depth, the next state is not stored
        // If it is not the first iteration of this perft, then there is something stored at this location
        free(achievedStates[nbMoveMade + 1].boardArray);
      }

      achievedStates[nbMoveMade + 1] = newState;
    }

    u64 moveOutput = perft(depth - 1, achievedStates, maximumDepth, firstMoveOfMaxDepth && i == 0); // We generate the moves for the next perft
    // if (depth == maximumDepth) {
    //   printMoveToAlgebraic(move_list.items[i]);
    //   printf(": %lld\n", moveOutput);
    // }
    nodes += moveOutput;
  }
  
  return nodes;
}

char* startingFenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// To compile the perft program: gcc -g -o perft testing/perft.c src/chessGameEmulator.c testing/logChessStructs.c src/fenString.c src/moveGenerator.c
// To run the compiled program: ./perft <depth>
// To check for memory leaks that program: valgrind --leak-check=full --track-origins=yes ./perft <depth>
int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Usage: ./perft <depth>\n");
    return 0;
  }

  int maximumDepth = atoi(argv[1]);

  if (maximumDepth <= 0) {
    printf("Invalid depth %d\n", maximumDepth);
    return 0;
  }

  GameState achievedStates[maximumDepth];
  
  GameState* startingState = setGameStateFromFenString(
    startingFenString
    , NULL);

  achievedStates[0] = *startingState;
  free(startingState);

  double averageExecutionTime = 0;
  clock_t begin, end;
  u64 perftResult;

  for (int iterations = 0; iterations < TEST_ITERATION; iterations++) {
    begin = clock();
    perftResult = perft(maximumDepth, achievedStates, maximumDepth, true);
    end = clock();
    double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
    averageExecutionTime += timeSpent;
    printf("ITERATION #%d, Time: %fs, Perft: %llu\n", iterations, timeSpent, perftResult);
    for (int i = 1; i < maximumDepth; i++) {
      // i starts at 1 because I do not want to free the first state
      free(achievedStates[i].boardArray);
    }
  }
  averageExecutionTime /= TEST_ITERATION;
  printf("Perft depth %d took on average %fms (%fs)\n", maximumDepth, averageExecutionTime * 1000, averageExecutionTime);
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

