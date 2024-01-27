#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "ChessGameEmulator.h"
#include "../testing/LogChessStructs.h"

void _updateCastlePerm(int pieceToMove, int from, ChessPosition* state) {
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

void _updateFiftyMoveRule(int pieceToMove, int to, ChessPosition* state) {
  if (pieceType(pieceToMove) == PAWN || pieceAtIndex(state->board, to) != NOPIECE) {
    state->turnsForFiftyRule = 0; // A pawn has moved or a capture has happened
  } else {
    state->turnsForFiftyRule++; // No captures or pawn advance happenned
  }
}

void playMove(Move move, ChessGame* game) {
  ChessPosition* state = game->currentState;
  Board* board = &state->board; 
  PieceCharacteristics colorToGo = state->colorToGo;
  int from = fromSquare(move);
  int to = toSquare(move);
  Flag flag = flagFromMove(move);
  Piece pieceToMove = pieceAtIndex(*board, from);

  u64 newZobristKey = state->key;

  newZobristKey ^= zobristRandomNumber->castlingPerms[(int) state->castlingPerm]; // Removing old castling perm
  _updateCastlePerm(pieceToMove, from, state);
  newZobristKey ^= zobristRandomNumber->castlingPerms[(int) state->castlingPerm]; // Adding new castling perm
  
  _updateFiftyMoveRule(pieceToMove, to, state);

  // Handling move
  togglePieceAtIndex(board, from, pieceToMove);
  newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][from]; // Removing piece to move
  
  Piece capturePiece = pieceAtIndex(*board, to);
  if (capturePiece != NOPIECE) { // A piece has been captured!
    togglePieceAtIndex(board, to, capturePiece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) capturePiece - 9][to]; // Removing captured piece
  }
  
  togglePieceAtIndex(board, to, pieceToMove);
  newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][to]; // Adding piece to move

  PieceCharacteristics oppositeColor = colorToGo == WHITE ? BLACK : WHITE;

  // Handling flags
  int rookIndex;
  Piece piece;
  int enPassantPawnIndex;
  switch (flag) {
  case NOFlAG:
    // To be able to detect invalid flags
    break;
  case EN_PASSANT:
    enPassantPawnIndex = colorToGo == WHITE ? to + 8 : to - 8;
    piece = pieceAtIndex(*board, enPassantPawnIndex);
    togglePieceAtIndex(board, enPassantPawnIndex, piece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][enPassantPawnIndex]; // Removing captured en-passant pawn
    break;
  case DOUBLE_PAWN_PUSH:
    enPassantPawnIndex = colorToGo == WHITE ? to + 8 : to - 8;
    state->enPassantTargetSquare = enPassantPawnIndex;
    break;
  case KING_SIDE_CASTLING:
    rookIndex = from + 3;
    piece = pieceAtIndex(*board, rookIndex);
    togglePieceAtIndex(board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][rookIndex]; // Removing rook
    togglePieceAtIndex(board, to - 1, piece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][to - 1]; // Adding rook
    break;
  case QUEEN_SIDE_CASTLING:
    rookIndex = from - 4;
    piece = pieceAtIndex(*board, rookIndex);
    togglePieceAtIndex(board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][rookIndex]; // Removing rook
    togglePieceAtIndex(board, to + 1, piece);
    newZobristKey ^= zobristRandomNumber->pieces[(int) piece - 9][to + 1]; // Adding rook
    break;
  case PROMOTE_TO_QUEEN: 
    togglePieceAtIndex(board, to, makePiece(colorToGo, QUEEN));
    newZobristKey ^= zobristRandomNumber->pieces[(int) makePiece(colorToGo, QUEEN) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_KNIGHT: 
    togglePieceAtIndex(board, to, makePiece(colorToGo, KNIGHT));
    newZobristKey ^= zobristRandomNumber->pieces[(int) makePiece(colorToGo, KNIGHT) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_ROOK: 
    togglePieceAtIndex(board, to, makePiece(colorToGo, ROOK));
    newZobristKey ^= zobristRandomNumber->pieces[(int) makePiece(colorToGo, ROOK) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_BISHOP: 
    togglePieceAtIndex(board, to, makePiece(colorToGo, BISHOP));
    newZobristKey ^= zobristRandomNumber->pieces[(int) makePiece(colorToGo, BISHOP) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber->pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  default:
    printf("ERROR: Invalid flag %d\n", flag);
    printBoard(game->currentState->board);
    return;
  }
  
  if (flag != DOUBLE_PAWN_PUSH && state->enPassantTargetSquare != 0) {
    state->enPassantTargetSquare = 0;
  }
  newZobristKey ^= zobristRandomNumber->enPassantFile[state->enPassantTargetSquare % 8];

  state->colorToGo = oppositeColor;
  if (state->colorToGo == WHITE) {
    state->nbMoves++; // Only recording full moves
  }
  newZobristKey ^= zobristRandomNumber->sideToMove;

  // Adding previous zobrist key to the previousStates
  if (game->previousStatesCount >= game->previousStatesCapacity) {
    game->previousStatesCapacity *= 2;
    game->previousStates = realloc(game->previousStates, game->previousStatesCapacity * sizeof(ZobristKey));
    assert(game->previousStates != NULL && "Buy more RAM lol");
  }
  game->previousStates[game->previousStatesCount++] = state->key;

  // Updating the zobrist key
  state->key = newZobristKey;
}