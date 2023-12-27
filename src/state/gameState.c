#include "GameState.h"
#include "Constants.h"

#include "stdlib.h"
#include "assert.h"

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
    int *boardArray,
    int colorToGo, 
    int castlingPerm, 
    int enPassantTargetSquare, 
    int turnsForFiftyRule, 
    int nbMoves) {
    GameState* result = malloc(sizeof(GameState));
    assert(result != NULL && "Malloc failed so buy more RAM lol");
    Board board = { .boardArray = malloc(sizeof(int) * BOARD_SIZE) };
    assert(board.boardArray != NULL && "Malloc failed so buy more RAM lol");
    result->board = board;
    for (int i = 0; i < BOARD_SIZE; i++) {
        result->board.boardArray[i] = boardArray[i];
    }
    result->castlingPerm = castlingPerm;
    result->colorToGo = colorToGo;
    result->enPassantTargetSquare = enPassantTargetSquare;
    result->nbMoves = nbMoves;
    result->turnsForFiftyRule = turnsForFiftyRule;
    return result;
}
