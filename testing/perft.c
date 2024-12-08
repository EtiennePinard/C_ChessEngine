#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include "../src/magicBitBoard/MagicBitBoard.h"
#include "../src/MoveGenerator.h"
#include "../src/utils/FenString.h"
#include "../src/ChessGameEmulator.h"
#include "../src/state/ZobristKey.h"
#include "LogChessStructs.h"

#define TEST_ITERATION 100
#define MAXIMUM_DEPTH 20

int maximumDepth;
ChessGame game = { 0 };
ChessPosition posHistory[MAXIMUM_DEPTH] = { 0 };

bool divide = true;

u64 perft(int depth) {
  if (depth == 0) { return 1; }

  int nbOfMoves;
  Move validMoves[256];
  getValidMoves(validMoves, &nbOfMoves, game.currentPosition); // We do not care about draw by repetition
  u64 nodes = 0;

  // Note that here we take into account the fifty move fule
  // This could make it so that our perft result differ from other engine
  // Thus let us be aware of this potential bug
  if (nbOfMoves == 0) {
      // There is a checkmate, continuing to next move
      return 0;
  }

  if (!divide && depth == 1) {
    return nbOfMoves;
  }
  
  posHistory[maximumDepth - depth] = game.currentPosition;

  for (int moveIndex = 0; moveIndex < nbOfMoves; moveIndex++) {

    Move move = validMoves[moveIndex];

    playMove(move, &game); // Move is made

    u64 moveOutput = perft(depth - 1); // We generate the moves for the next perft

    if (divide && depth == maximumDepth) {
      printMoveToAlgebraic(move);
      printf(": %lu\n", moveOutput);
    }
    nodes += moveOutput;

    game.previousPositionsCount--;
    memcpy(&game.currentPosition, &posHistory[maximumDepth - depth], sizeof(ChessPosition));
  }
  
  return nodes;
}

bool isStringValidPerftNumber(char* string) {
  int index = 0;
  char currentChar;
  bool result = true;
  while ((currentChar = *(string + index))) {
    if (currentChar < '0' || currentChar > '9') {
      result = false;
      break;
    }
    index++;
  }
  return result;
}

/* Testing all the double pawn push case

rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 (Normal double pawn push) (ans: 20)
2r4k/8/8/8/3p4/8/2P5/2K5 w - - 0 1 (Pawn is pinned but still can double pawn push) (ans: 6)
7k/8/8/8/3p4/8/r1P1K3/8 w - - 0 1 (Pawn is pinned and cannot double pawn push) (ans: 7)
7k/8/8/4b3/8/8/3P4/K7 w - - 0 1 (King is in check and pawn can double push to get out of check) (ans: 3)

*/

/* Testing all the en-passant case

k6r/8/8/K1Pp4/8/8/8/8 w - d6 0 1 (Normal en-passant)
r7/7k/8/K1pP4/8/8/8/8 w - c6 0 1 (King is in check but en-passant is possible)
r3b3/7k/8/2pP4/1K6/8/8/2rb4 w - c6 0 1 (king is in check and en-passant will remove the check)
k7/b7/8/2Pp4/3K4/8/8/8 w - d6 0 1 (Pawn is pinned but en-passant is possible)
k7/8/8/K1Pp3r/8/8/8/8 w - d6 0 1 (Pawn is en-passant pinned)
8/8/3p4/KPp4r/1R2PpPk/8/8/ b - e3 0 1 (Pawn looks like it is en-passant pinned but it isn't) (ans: 16)
8/8/8/KPpP3r/1R3p1k/8/6P1/ w - c6 1 3 (Two pawns look en-passant pinned but both can do en-passant) (ans: 17)

*/

/**
 * Represents a position to perform a perft test on.
 * The nbTest parameter indicates for how many depths there is a results
 * perftResults returns the expected number of moves for a depth (the indices of said int)
*/
typedef struct testPosition {
  char* fenString;
  int nbTest;
  int* perftResults;
} TestPosition;

// These are the 8 ANSI color types
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define NUM_TEST_POSITIONS 6

void test() {
  magicBitBoardInitialize();
  zobristKeyInitialize();
  
  int biggestDepth = 5;
  if (biggestDepth > MAXIMUM_DEPTH) {
    printf("The biggest depth of test %d exceeds the position history limit, %d\n", biggestDepth, MAXIMUM_DEPTH);
    magicBitBoardTerminate();
    exit(EXIT_FAILURE);
  }

  int startingPosResults[6] = {1, 20, 400, 8902, 197281, 4865609};
  TestPosition startingPositionTests = {
    .fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 
    .nbTest = 6, 
    .perftResults = startingPosResults
  };
  
  int pos2Result[5] = {1, 48, 2039, 97862, 4085603};
  TestPosition pos2 = {
    .fenString = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    .nbTest = 5,
    .perftResults = pos2Result
  };
  
  int pos3Result[6] = {1, 14, 191, 2812, 43238, 674624};
  TestPosition pos3 = {
    .fenString = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    .nbTest = 6,
    .perftResults = pos3Result
  };

  int pos4Result[5] = {1, 6, 264, 9467, 422333};
  TestPosition pos4 = {
    .fenString = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    .nbTest = 5,
    .perftResults = pos4Result
  };

  int pos5Result[5] = {1, 44, 1486, 62379, 2103487};
  TestPosition pos5 = {
    .fenString = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    .nbTest = 5,
    .perftResults = pos5Result
  };

  int pos6Result[5] = {1, 46, 2079, 89890, 3894594};
  TestPosition pos6 = {
    .fenString = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    .nbTest = 5,
    .perftResults = pos6Result
  };

  TestPosition testPositions[NUM_TEST_POSITIONS] = { 
    startingPositionTests, 
    pos2,
    pos3,
    pos4,
    pos5,
    pos6, 
  };
  
  game.currentPosition = (ChessPosition) { 0 };
  game.whiteTimeMs = 0; // We don't care about time in perft
  game.blackTimeMs = 0;
  game.previousPositionsCount = 0;

  ChessPosition startingState;
  u64 perftResult;
  double timeSpent;
  clock_t begin, end;
  char* testPassed = GRN "✓" RESET;
  char* testFailedPrefix = RED "❌" RESET " Test failed (expected ";

  clock_t fullTestBegin = clock();
  
  for (int i = 0; i < NUM_TEST_POSITIONS; i++) {
    TestPosition testPosition = testPositions[i];
    
    setChessPositionFromFenString(testPosition.fenString, &game.currentPosition);
    startingState = game.currentPosition;

    printf(RESET "Running test for fen string: %s\n", testPosition.fenString);

    for (int depth = 0; depth < testPosition.nbTest; depth++) {
      maximumDepth = depth;
      begin = clock();
      perftResult = perft(depth);
      end = clock();
      timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
      
      printf(RESET "Depth: " GRN "%d " RESET "ply  " RESET "Result: " RED "%lu" RESET "  Time: " BLU "%f " RESET "ms ", depth, perftResult, timeSpent * 1000);
      if (perftResult == (u64) testPosition.perftResults[depth]) {
        printf("%s" RESET "\n", testPassed);
      } else {
        printf("%s " RED "%d" RESET ")\n", testFailedPrefix, testPosition.perftResults[depth]);
      }

      memcpy(&game.currentPosition, &startingState, sizeof(ChessPosition));
      game.previousPositionsCount = 0;
    }
 
    printf("\n");
  }

  clock_t fullTestEnd = clock();
  double fullTestTimeSpent = (double)(fullTestEnd - fullTestBegin) / CLOCKS_PER_SEC;
  printf(RESET "The full test took " BLU "%f " RESET "ms" RESET "\n", fullTestTimeSpent * 1000);

  magicBitBoardTerminate();
}

// To compile and run the program: ./perft
// To check for memory leaks: valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./perftTesting <args>
int main(int argc, char* argv[]) {
  if (argc == 1) {
    printf("Usage: ./%s <mode (divide, time, test)> [position (fen string)] [depth (positive integer)]\n", argv[0]);
    printf("\tIf `mode` is not provided it will default to divide mode\n");
    printf("\tIf `position` is not provided it will default to the starting position\n");
    printf("\tThe `position` and `depth` argument only apply for the divide and time mode\n");
    printf("\t`depth` needs to be provided for the modes it applies to\n");
    exit(EXIT_FAILURE);
  }

  char* fenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  maximumDepth = -1;

  { // I am wrapping this code in a scope to not "leak out" the firstArg variable  
    char* firstArg = argv[1];
    if (strcmp(firstArg, "test") == 0) {
      divide = false;
      test();
      return 0;
    }
    if (strcmp(firstArg, "time") == 0) {
      divide = false;
    } else if (strcmp(firstArg, "divide") != 0) {
      // No parameter is provided, so it is either a fen string of a depth
      if (isStringValidPerftNumber(firstArg)) {
        // No fen string is provided, so the first argument is just the depth
        maximumDepth = atoi(firstArg);
      } else {
        // The first argument is a fen string
        fenString = firstArg;
      }
    }
  }

  if (argc >= 3) {
    char* secondArg = argv[2];
    if (isStringValidPerftNumber(secondArg)) {
      // No fen string is provided, so the first argument is just the depth
      maximumDepth = atoi(secondArg);
    } else {
      // The first argument is a fen string
      fenString = secondArg;
    }
  }

  if (argc >= 4) {
    char* thirdArgument = argv[3];
    // This argument needs to be the depth
    if (!isStringValidPerftNumber(thirdArgument)) {
      printf("The argument %s is not a valid perft number\n", thirdArgument);
      exit(EXIT_FAILURE);
    } else {
      maximumDepth = atoi(thirdArgument);
    }
  }

  if (maximumDepth < 0) {
    printf("You did not provide a valid depth for the mode `%s`\n", divide ? "divide" : "time");
    exit(EXIT_FAILURE);
  }

  magicBitBoardInitialize();
  zobristKeyInitialize();

  ChessPosition currentPosition = { 0 };
  // Note: We don't care about time in perft
  if (!setupChesGame(&game, &currentPosition, fenString, (u32) 0, (u32) 0)) { 
    printf("ERROR while setup of chess game state\n Exiting\n");
    exit(EXIT_FAILURE);
  }

  assert(posHistory != NULL && "Buy more RAM lol");

  u64 perftResult;

  if (divide) {
    printBoard(game.currentPosition.board);
    perftResult = perft(maximumDepth);
    printf("Perft depth %d returned a total number of moves of %lu\n", maximumDepth, perftResult);
  } else {
    ChessPosition startingState = game.currentPosition;

    double averageExecutionTime = 0;
    clock_t begin, end;
    for (int iterations = 0; iterations < TEST_ITERATION; iterations++) {
      begin = clock();
      perftResult = perft(maximumDepth);
      end = clock();
      
      double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;
      averageExecutionTime += timeSpent;
      
      memcpy(&game.currentPosition, &startingState, sizeof(ChessPosition));
      game.previousPositionsCount = 0;

      printf("ITERATION #%d, Time: %fs, Perft: %lu\n", iterations, timeSpent, perftResult);
    }
    averageExecutionTime /= TEST_ITERATION;
    printf("Perft depth %d took on average %fms (%fs)\n", maximumDepth, averageExecutionTime * 1000, averageExecutionTime);
  }

  magicBitBoardTerminate();
  return 0;
}