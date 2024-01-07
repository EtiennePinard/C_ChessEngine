#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "ChessGameEmulator.h"

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

void _updateFiftyMoveRule(int pieceToMove, int to, GameState* state) {
  if (pieceType(pieceToMove) == PAWN || pieceAtIndex(state->board, to) != NOPIECE) {
    state->turnsForFiftyRule = 0; // A pawn has moved or a capture has happened
  } else {
    state->turnsForFiftyRule++; // No captures or pawn advance happenned
  }
}

void makeMove(Move move, GameState* state) {
  int from = fromSquareFromMove(move);
  int to = toSquareFromMove(move);
  Flag flag = flagFromMove(move);
  int pieceToMove = pieceAtIndex(state->board, from);
  _updateCastlePerm(pieceToMove, from, state);
  _updateFiftyMoveRule(pieceToMove, to, state);

  handleMove(&state->board, from, to);

  PieceCharacteristics oppositeColor = state->colorToGo == WHITE ? BLACK : WHITE;

  int rookIndex;
  Piece piece;
  int enPassantPawnIndex;
  switch (flag) {
  case NOFlAG:
    // Just for completeness
    break;
  case EN_PASSANT:
    enPassantPawnIndex = state->colorToGo == WHITE ? to + 8 : to - 8;
    piece = pieceAtIndex(state->board, enPassantPawnIndex);
    togglePieceAtIndex(&state->board, enPassantPawnIndex, piece);
    break;
  case DOUBLE_PAWN_PUSH:
    enPassantPawnIndex = state->colorToGo == WHITE ? to + 8 : to - 8;
    state->enPassantTargetSquare = enPassantPawnIndex;
    break;
  case KING_SIDE_CASTLING:
    rookIndex = from + 3;
    piece = pieceAtIndex(state->board, rookIndex);
    togglePieceAtIndex(&state->board, rookIndex, piece);
    togglePieceAtIndex(&state->board, to - 1, piece);
    break;
  case QUEEN_SIDE_CASTLING:
    rookIndex = from - 4;
    piece = pieceAtIndex(state->board, rookIndex);
    togglePieceAtIndex(&state->board, rookIndex, piece);
    togglePieceAtIndex(&state->board, to + 1, piece);
    break;
  case PROMOTE_TO_QUEEN: 
    togglePieceAtIndex(&state->board, to, makePiece(state->colorToGo, QUEEN));
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&state->board, to, pieceToMove);
    break;
  case PROMOTE_TO_KNIGHT: 
    togglePieceAtIndex(&state->board, to, makePiece(state->colorToGo, KNIGHT));
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&state->board, to, pieceToMove);
    break;
  case PROMOTE_TO_ROOK: 
    togglePieceAtIndex(&state->board, to, makePiece(state->colorToGo, ROOK));
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&state->board, to, pieceToMove);
    break;
  case PROMOTE_TO_BISHOP: 
    togglePieceAtIndex(&state->board, to, makePiece(state->colorToGo, BISHOP));
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&state->board, to, pieceToMove);
    break;
  default:
    printf("ERROR: Invalid flag %d\n", flag);
    return;
  }
  state->colorToGo = oppositeColor;
  if (flag != DOUBLE_PAWN_PUSH && state->enPassantTargetSquare != -1) {
    state->enPassantTargetSquare = -1;
  }
  if (state->colorToGo == WHITE) {
    state->nbMoves++; // Only recording full moves
  }
}