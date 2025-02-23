#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "ChessGameEmulator.h"
#include "../state/ZobristKey.h"
#include "../utils/Math.h"

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
        state->castlingPerm &= 0b1011; // Removing white queen side castling perm
      }
      break;

    case BLACK | ROOK:
      if (from == kingSideBlackRookPosition) {
        state->castlingPerm &= 0b1101; // Removing black king side castling perm
      } else if (from == queenSideBlackRookPosition) {
        state->castlingPerm &= 0b1110; // Removing black queen side castling perm
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
  if (Piece_type(pieceToMove) == PAWN || Board_pieceAtIndex(state->board, to) != NOPIECE) {
    state->turnsForFiftyRule = 0; // A pawn has moved or a capture has happened
  } else {
    state->turnsForFiftyRule++; // No captures or pawn advance happened
  }
}

// Note: The __attribute__ ((unused)) is there so that gcc don't put us a warning if we don't include the repetition table
void Engine_playMove(Move move, ChessPosition* position, __attribute__ ((unused)) bool storePositionInRepetitionTable) {
  PieceCharacteristics colorToGo = position->colorToGo;
  int from = Move_fromSquare(move);
  int to = Move_toSquare(move);
  Flag flag = Move_flag(move);
  Piece pieceToMove = Board_pieceAtIndex(position->board, from);

  u64 newZobristKey = position->key;

  newZobristKey ^= zobristRandomNumber.castlingPerms[(int) position->castlingPerm]; // Removing old castling perm
  _updateCastlePerm(pieceToMove, from, position);
  newZobristKey ^= zobristRandomNumber.castlingPerms[(int) position->castlingPerm]; // Adding new castling perm
  
  _updateFiftyMoveRule(pieceToMove, to, position);

  // Handling move
  Board_togglePieceAtIndex(&position->board, from, pieceToMove);
  newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][from]; // Removing piece to move
  
  Piece capturePiece = Board_pieceAtIndex(position->board, to);
  if (capturePiece != NOPIECE) { // A piece has been captured!
    Board_togglePieceAtIndex(&position->board, to, capturePiece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) capturePiece - 9][to]; // Removing captured piece
  }
  
  Board_togglePieceAtIndex(&position->board, to, pieceToMove);
  newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Adding piece to move

  PieceCharacteristics oppositeColor = colorToGo == WHITE ? BLACK : WHITE;

  // Handling enPassant
  int zobristRandomNumberEnPassantIndex = file(position->enPassantTargetSquare);
  if (position->enPassantTargetSquare > 0) { zobristRandomNumberEnPassantIndex++; }
  newZobristKey ^= zobristRandomNumber.enPassantFile[zobristRandomNumberEnPassantIndex];
  position->enPassantTargetSquare = 0;

  // Handling flags
  int rookIndex;
  Piece piece;
  int enPassantPawnIndex;
  switch (flag) {
  case NOFLAG:
    // To be able to detect invalid flags
    break;
  case EN_PASSANT:
    enPassantPawnIndex = colorToGo == WHITE ? to + 8 : to - 8;
    piece = Board_pieceAtIndex(position->board, enPassantPawnIndex);
    Board_togglePieceAtIndex(&position->board, enPassantPawnIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][enPassantPawnIndex]; // Removing captured en-passant pawn
    break;
  case DOUBLE_PAWN_PUSH:
    enPassantPawnIndex = colorToGo == WHITE ? to + 8 : to - 8;
    position->enPassantTargetSquare = enPassantPawnIndex;
    newZobristKey ^= zobristRandomNumber.enPassantFile[file(enPassantPawnIndex) + 1];
    break;
  case KING_SIDE_CASTLING:
    rookIndex = from + 3;
    piece = Board_pieceAtIndex(position->board, rookIndex);
    Board_togglePieceAtIndex(&position->board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][rookIndex]; // Removing rook
    Board_togglePieceAtIndex(&position->board, to - 1, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][to - 1]; // Adding rook
    break;
  case QUEEN_SIDE_CASTLING:
    rookIndex = from - 4;
    piece = Board_pieceAtIndex(position->board, rookIndex);
    Board_togglePieceAtIndex(&position->board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][rookIndex]; // Removing rook
    Board_togglePieceAtIndex(&position->board, to + 1, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][to + 1]; // Adding rook
    break;
  case PROMOTE_TO_QUEEN: 
    Board_togglePieceAtIndex(&position->board, to, Piece_makePiece(colorToGo, QUEEN));
    newZobristKey ^= zobristRandomNumber.pieces[(int) Piece_makePiece(colorToGo, QUEEN) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    Board_togglePieceAtIndex(&position->board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_KNIGHT: 
    Board_togglePieceAtIndex(&position->board, to, Piece_makePiece(colorToGo, KNIGHT));
    newZobristKey ^= zobristRandomNumber.pieces[(int) Piece_makePiece(colorToGo, KNIGHT) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    Board_togglePieceAtIndex(&position->board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_ROOK: 
    Board_togglePieceAtIndex(&position->board, to, Piece_makePiece(colorToGo, ROOK));
    newZobristKey ^= zobristRandomNumber.pieces[(int) Piece_makePiece(colorToGo, ROOK) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    Board_togglePieceAtIndex(&position->board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_BISHOP: 
    Board_togglePieceAtIndex(&position->board, to, Piece_makePiece(colorToGo, BISHOP));
    newZobristKey ^= zobristRandomNumber.pieces[(int) Piece_makePiece(colorToGo, BISHOP) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    Board_togglePieceAtIndex(&position->board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  default:
    printf("ERROR: Invalid flag %d\n", flag);
    return;
  }
  
  position->colorToGo = oppositeColor;
  if (position->colorToGo == WHITE) {
    position->nbMoves++; // Only recording full moves
  }
  newZobristKey ^= zobristRandomNumber.sideToMove;

  // Storing the old Zobrist key only if the repetitionTable.c file is included
  #ifdef RepetitionTable_storeKey
  if (storePositionInRepetitionTable) {
    RepetitionTable_storeKey(position->key);
  }
  #endif

  // Updating the zobrist key
  position->key = newZobristKey;
}