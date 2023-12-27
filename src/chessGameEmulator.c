#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "chessGameEmulator.h"

void _updateCastlePerm(int pieceToMove, int from, GameState* state) {
  if (state->castlingPerm == 0) { return; }

  int kingSideWhiteRookPosition = 63;
  int queenSideWhiteRookPosition = 56;
  int kingSideBlackRookPosition = 7;
  int queenSideBlackRookPosition = 0;
  
  switch (pieceToMove) {

    case WHITE | ROOK:
      if (from == kingSideWhiteRookPosition) {
        state->castlingPerm &= 0b0111; // Removing white king side castling perm
      } else if (from == queenSideWhiteRookPosition) {
        state->castlingPerm &= 0b1011; // Removing white queen side caslting perm
      }
      break;

    case BLACK | ROOK:
      if (from == kingSideBlackRookPosition) {
        state->castlingPerm &= 0b1101; // Removing black king side caslting perm
      } else if (from == queenSideBlackRookPosition) {
        state->castlingPerm &= 0b1110; // Removing black queen side caslting perm
      }
      break;

    case WHITE | KING:
      state->castlingPerm &= 0b0011; // The white king castling perm is none existant
      break;

    case BLACK | KING:
      state->castlingPerm &= 0b1100; // The black king castling perm is none existant
      break;

    default:
      break;
  }

}

void _updateFiftyFiftyMove(int pieceToMove, int to, GameState* state) {
  if (pieceType(pieceToMove) == PAWN) {
    state->turnsForFiftyRule = 0; // A pawn has moved
  } else if (state->board.boardArray[to] != NONE) {
    state->turnsForFiftyRule = 0; // There has been a capture
  }
}

void makeMove(Move move, GameState* state) {
  int from = fromSquareFromMove(move);
  int to = toSquareFromMove(move);
  Flag flag = flagFromMove(move);
  int pieceToMove = state->board.boardArray[from];
  _updateCastlePerm(pieceToMove, from, state);
  _updateFiftyFiftyMove(pieceToMove, to, state);

  state->board.boardArray[from] = NONE;
  if (state->board.boardArray[to] != NONE) {
    state->turnsForFiftyRule++;
  }
  state->board.boardArray[to] = pieceToMove;
  int rookIndex;
  int rookPiece;
  int enPassantTargetSquare;
  int pawnIndex;
  switch (flag) {
  case NOFlAG:
    // Just for completeness
    break;
  case EN_PASSANT:
    pawnIndex = state->colorToGo == WHITE ? to + 8 : to - 8;
    state->board.boardArray[pawnIndex] = NONE;
    break;
  case DOUBLE_PAWN_PUSH:
    enPassantTargetSquare = state->colorToGo == WHITE ? to + 8 : to - 8;
    state->enPassantTargetSquare = enPassantTargetSquare;
    break;
  case KING_SIDE_CASTLING:
    rookIndex = from + 3;
    rookPiece = state->board.boardArray[rookIndex];
    state->board.boardArray[rookIndex] = NONE;
    state->board.boardArray[to - 1] = rookPiece;
    break;
  case QUEEN_SIDE_CASTLING:
    rookIndex = from - 4;
    rookPiece = state->board.boardArray[rookIndex];
    state->board.boardArray[rookIndex] = NONE;
    state->board.boardArray[to + 1] = rookPiece;
    break;
  case PROMOTE_TO_QUEEN: 
    state->board.boardArray[to] = state->colorToGo | QUEEN;
    break;
  case PROMOTE_TO_KNIGHT: 
    state->board.boardArray[to] = state->colorToGo | KNIGHT;
    break;
  case PROMOTE_TO_ROOK: 
    state->board.boardArray[to] = state->colorToGo | ROOK;
    break;
  case PROMOTE_TO_BISHOP: 
    state->board.boardArray[to] = state->colorToGo | BISHOP;
    break;
  default:
    printf("ERROR: Invalid flag %d\n", flag);
    return;
  }
  state->colorToGo = state->colorToGo == WHITE ? BLACK : WHITE;
  if (flag != DOUBLE_PAWN_PUSH && state->enPassantTargetSquare != -1) {
    state->enPassantTargetSquare = -1;
  }
  state->turnsForFiftyRule++;
  if (state->colorToGo == WHITE) {
    state->nbMoves++; // Only recording full moves
  }
}