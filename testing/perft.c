#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/MoveGenerator.h"
#include "../src/utils/FenString.h"
#include "../src/ChessGameEmulator.h"
#include "LogChessStructs.h"

#define TEST_ITERATION 100

int maximumDepth;
GameState* achievedStates;

bool debug = true;

u64 perft(int depth) {
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

  if (!debug && depth == 1) {
    return nbOfMoves;
  }

  for (i = 0; i < nbOfMoves; i++) {

    GameState newState = previousState;
    Move move = moves[i];
    makeMove(move, &newState); // Move is made
    
    if (depth != 1) { // when depth == 1 it would write to invalid indices
      achievedStates[nbMoveMade + 1] = newState;
    }

    u64 moveOutput = perft(depth - 1); // We generate the moves for the next perft
    if (debug && depth == maximumDepth) {
      printMoveToAlgebraic(move);
      printf(": %lu\n", moveOutput);
    }
    nodes += moveOutput;
  }
  
  return nodes;
}

bool isStringValidPerftNumber(char* string) {
  int index = 0;
  char currentChar;
  bool result = true;
  while (currentChar = *(string + index)) {
    if (currentChar < '0' || currentChar > '9') {
      result = false;
      break;
    }
    index++;
  }
  return result;
}

// k6r/8/8/K1Pp4/8/8/8/8 w - d6 0 1 (Normal en-passant)
// r7/7k/8/K1pP4/8/8/8/8 w - c6 0 1 (King is in check but en-passant is possible)
// r3b3/7k/8/2pP4/1K6/8/8/2rb4 w - c6 0 1 (king is in check and en-passant will remove the check)
// k7/b7/8/2Pp4/3K4/8/8/8 w - d6 0 1 (Pawn is pinned but en-passant is possible)
// k7/8/8/K1Pp3r/8/8/8/8 w - d6 0 1 (Pawn is en-passant pinned)
// TODO: Add a test functionnality for the perft program
// To compile and run the program: ./perft <depth>
// To check for memory leaks that program: valgrind --leak-check=full --track-origins=yes -s ./perftTesting <depth>
int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Usage: ./%s <depth (num)> [position (fen string)] [mode (debug or time)]\n", argv[0]);
    printf("Note that order does not matter for `position` and `mode` arguments, and they are optional\n");
    exit(EXIT_FAILURE);
  }

  if (isStringValidPerftNumber(argv[1])) {
    maximumDepth = atoi(argv[1]);
  } else {
    printf("Argument %s is not a valid digit\n", argv[1]);
  } 
  if (maximumDepth <= 0) {
    printf("Invalid depth %d\n", maximumDepth);
    exit(EXIT_FAILURE);
  }

  // The default is the starting fen string
  char* fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // Handling position and debug
  if (argc >= 3) {
    char* secondArg = argv[2];
    if (strcmp(secondArg, "time") == 0) {
      debug = false;
    } else if (strcmp(secondArg, "debug")) {
      fenString = secondArg;
    }
  }
  if (argc >= 4) {
    char* secondArg = argv[3];
    if (strcmp(secondArg, "time") == 0) {
      debug = false;
    } else if (strcmp(secondArg, "debug")) {
      fenString = secondArg;
    }
  }

  magicBitBoardInitialize();
  
  achievedStates = malloc(sizeof(GameState) * maximumDepth);
  
  GameState startingState = { 0 }; 
  
  if (!setGameStateFromFenString(fenString, &startingState)) {
    printf("The fen string \"%s\" is invalid!\n", fenString);
    exit(EXIT_FAILURE);
  }

  achievedStates[0] = startingState;

  u64 perftResult;

  if (debug) {
    printBoard(startingState.board);
    perftResult = perft(maximumDepth);
    printf("Perft depth %d returned a total number of moves of %lu\n", maximumDepth, perftResult);
  } else {
    double averageExecutionTime = 0;
    clock_t begin, end;
    for (int iterations = 0; iterations < TEST_ITERATION; iterations++) {
      begin = clock();
      perftResult = perft(maximumDepth);
      end = clock();
      double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
      averageExecutionTime += timeSpent;
      printf("ITERATION #%d, Time: %fs, Perft: %lu\n", iterations, timeSpent, perftResult);
    }
    averageExecutionTime /= TEST_ITERATION;
    printf("Perft depth %d took on average %fms (%fs)\n", maximumDepth, averageExecutionTime * 1000, averageExecutionTime);
  }
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

