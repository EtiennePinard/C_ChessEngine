#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../src/moveGenerator.h"
#include "../src/fenString.h"
#include "logChessStructs.h"
#include "chessGameEmulator.h"

typedef unsigned long long u64;

GameState copyState(GameState from) {
  GameState to = {
    .boardArray = (int*) malloc(sizeof(int) * BOARD_SIZE),
    .castlinPerm = from.castlinPerm,
    .colorToGo = from.colorToGo,
    .enPassantTargetSquare = from.enPassantTargetSquare,
    .nbMoves = from.nbMoves,
    .turnsForFiftyRule = from.turnsForFiftyRule
  };
  for (int i = 0; i < BOARD_SIZE; i++) {
    to.boardArray[i] = from.boardArray[i];
  }
  return to;
}


u64 perft(int depth, GameState* achievedStates, int maximumDepth, bool firstMoveOfMaxDepth) {
  if (depth == 0) { return 1; }
  int nbMoveMade = maximumDepth - depth;
  GameState previousState = achievedStates[nbMoveMade];
  Moves* move_list = getValidMoves(&previousState, achievedStates, nbMoveMade + 1);
  int n_moves, i;
  u64 nodes = 0;

  n_moves = move_list->count;
  if (n_moves == 1) {
    if ((move_list->items[0] >> 12) == STALEMATE || 
        (move_list->items[0] >> 12) == CHECKMATE ||
        (move_list->items[0] >> 12) == DRAW) {
          // Game is finished, continuing to next move
          free(move_list->items);
          free(move_list);
          return 0;
        }
  }

  if (depth == 1) {
    free(move_list->items);
    free(move_list);
    return n_moves;
  }

  for (i = 0; i < n_moves; i++) {
    if (depth != 1) { // So that if we want to check moves at depth 1 with don't get memory leaks
      GameState newState = copyState(previousState);
      makeMove(move_list->items[i], &newState); // Move is made

      if (i != 0 || !firstMoveOfMaxDepth) { // When it is the first time we have reached this depth, the next state is not stored
        // If it is not the first iteration of this perft, then there is something stored at this location
        free(achievedStates[nbMoveMade + 1].boardArray);
      }

      achievedStates[nbMoveMade + 1] = newState;
    }

    u64 moveOutput = perft(depth - 1, achievedStates, maximumDepth, firstMoveOfMaxDepth && i == 0); // We generate the moves for the next perft
    if (depth == maximumDepth) {
      printMoveToAlgebraic(move_list->items[i]);
      printf(": %lld\n", moveOutput);
    }
    nodes += moveOutput;
  }

  free(move_list->items);
  free(move_list);
  
  return nodes;
}

char* startingFenString = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// TODO: Test the move repetition strategy

// To compile the perft program: gcc -g -o perft testing/perft.c testing/chessGameEmulator.c testing/logChessStructs.c src/fenString.c src/moveGenerator.c
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

  u64 totalNumberOfMoves = perft(maximumDepth, achievedStates, maximumDepth, true);
  printf("Total moves made for perft %d is %llu\n", maximumDepth, totalNumberOfMoves);
  for (int i = 0; i < maximumDepth; i++) {
    free(achievedStates[i].boardArray);
  }
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

