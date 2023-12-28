#include "GameState.h"
#include "Constants.h"

#include "stdlib.h"
#include "assert.h"

size_t nbGameStatesInArray(GameState* gameStates) {
  size_t size = 0;
  // I just check that colorToGo == 0
  // If that is the case, then the GameState is not valid, as WHITE = 8 and BLACK = 16
  while (gameStates[size].colorToGo == 0) {
    size++;
  }
  return size;
}

GameState copyState(GameState from) {

  Board board = {
    .boardArray = (Piece*) malloc(sizeof(Piece) * BOARD_SIZE)
  };

  GameState to = {
    .board = board,
    .castlingPerm = from.castlingPerm,
    .colorToGo = from.colorToGo,
    .enPassantTargetSquare = from.enPassantTargetSquare,
    .nbMoves = from.nbMoves,
    .turnsForFiftyRule = from.turnsForFiftyRule
  };

  for (int i = 0; i < BOARD_SIZE; i++) {
    to.board.boardArray[i] = from.board.boardArray[i];
  }
  return to;
}

GameState* createState(
    Board board,
    int colorToGo, 
    int castlingPerm, 
    int enPassantTargetSquare, 
    int turnsForFiftyRule, 
    int nbMoves) {
    GameState* result = malloc(sizeof(GameState));
    assert(result != NULL && "Malloc failed so buy more RAM lol");
    result->board = board;
    result->castlingPerm = castlingPerm;
    result->colorToGo = colorToGo;
    result->enPassantTargetSquare = enPassantTargetSquare;
    result->nbMoves = nbMoves;
    result->turnsForFiftyRule = turnsForFiftyRule;
    return result;
}
