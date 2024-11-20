#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include "ChessGameEmulator.h"
#include "../src/state/ZobristKey.h"
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
  PieceCharacteristics colorToGo = game->currentState.colorToGo;
  int from = fromSquare(move);
  int to = toSquare(move);
  Flag flag = flagFromMove(move);
  Piece pieceToMove = pieceAtIndex(game->currentState.board, from);

  u64 newZobristKey = game->currentState.key;

  newZobristKey ^= zobristRandomNumber.castlingPerms[(int) game->currentState.castlingPerm]; // Removing old castling perm
  _updateCastlePerm(pieceToMove, from, &game->currentState);
  newZobristKey ^= zobristRandomNumber.castlingPerms[(int) game->currentState.castlingPerm]; // Adding new castling perm
  
  _updateFiftyMoveRule(pieceToMove, to, &game->currentState);

  // Handling move
  togglePieceAtIndex(&game->currentState.board, from, pieceToMove);
  newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][from]; // Removing piece to move
  
  Piece capturePiece = pieceAtIndex(game->currentState.board, to);
  if (capturePiece != NOPIECE) { // A piece has been captured!
    togglePieceAtIndex(&game->currentState.board, to, capturePiece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) capturePiece - 9][to]; // Removing captured piece
  }
  
  togglePieceAtIndex(&game->currentState.board, to, pieceToMove);
  newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Adding piece to move

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
    piece = pieceAtIndex(game->currentState.board, enPassantPawnIndex);
    togglePieceAtIndex(&game->currentState.board, enPassantPawnIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][enPassantPawnIndex]; // Removing captured en-passant pawn
    break;
  case DOUBLE_PAWN_PUSH:
    enPassantPawnIndex = colorToGo == WHITE ? to + 8 : to - 8;
    game->currentState.enPassantTargetSquare = enPassantPawnIndex;
    break;
  case KING_SIDE_CASTLING:
    rookIndex = from + 3;
    piece = pieceAtIndex(game->currentState.board, rookIndex);
    togglePieceAtIndex(&game->currentState.board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][rookIndex]; // Removing rook
    togglePieceAtIndex(&game->currentState.board, to - 1, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][to - 1]; // Adding rook
    break;
  case QUEEN_SIDE_CASTLING:
    rookIndex = from - 4;
    piece = pieceAtIndex(game->currentState.board, rookIndex);
    togglePieceAtIndex(&game->currentState.board, rookIndex, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][rookIndex]; // Removing rook
    togglePieceAtIndex(&game->currentState.board, to + 1, piece);
    newZobristKey ^= zobristRandomNumber.pieces[(int) piece - 9][to + 1]; // Adding rook
    break;
  case PROMOTE_TO_QUEEN: 
    togglePieceAtIndex(&game->currentState.board, to, makePiece(colorToGo, QUEEN));
    newZobristKey ^= zobristRandomNumber.pieces[(int) makePiece(colorToGo, QUEEN) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&game->currentState.board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_KNIGHT: 
    togglePieceAtIndex(&game->currentState.board, to, makePiece(colorToGo, KNIGHT));
    newZobristKey ^= zobristRandomNumber.pieces[(int) makePiece(colorToGo, KNIGHT) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&game->currentState.board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_ROOK: 
    togglePieceAtIndex(&game->currentState.board, to, makePiece(colorToGo, ROOK));
    newZobristKey ^= zobristRandomNumber.pieces[(int) makePiece(colorToGo, ROOK) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&game->currentState.board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  case PROMOTE_TO_BISHOP: 
    togglePieceAtIndex(&game->currentState.board, to, makePiece(colorToGo, BISHOP));
    newZobristKey ^= zobristRandomNumber.pieces[(int) makePiece(colorToGo, BISHOP) - 9][to]; // Adding promotion piece
    // To undo what the `handleMove` function did wrong (on accident, I might add)
    togglePieceAtIndex(&game->currentState.board, to, pieceToMove);
    newZobristKey ^= zobristRandomNumber.pieces[(int) pieceToMove - 9][to]; // Removing piece to move (it was added on accident)
    break;
  default:
    printf("ERROR: Invalid flag %d\n", flag);
    printBoard(game->currentState.board);
    return;
  }
  
  if (flag != DOUBLE_PAWN_PUSH && game->currentState.enPassantTargetSquare != 0) {
    game->currentState.enPassantTargetSquare = 0;
  }
  newZobristKey ^= zobristRandomNumber.enPassantFile[game->currentState.enPassantTargetSquare % 8];

  game->currentState.colorToGo = oppositeColor;
  if (game->currentState.colorToGo == WHITE) {
    game->currentState.nbMoves++; // Only recording full moves
  }
  newZobristKey ^= zobristRandomNumber.sideToMove;


  if (game->previousStatesCount > PREVIOUS_STATE_CAPACITY) {
    printf("***** RARE ACHIEVEMENT UNLOCKED ***** (or you have a bug in your code )\n");
    printf("Your game with %d moves has exceeded %d moves.\n", game->previousStatesCount, PREVIOUS_STATE_CAPACITY - 1);
    printf("Since this exceeds the number of zobrist key we can store the program will exit cold turkey\n");
    printf("Sorry about that...\n");

    printBoard(game->currentState.board);
    exit(EXIT_FAILURE);
  }

  game->previousStates[game->previousStatesCount++] = game->currentState.key;

  // Updating the zobrist key
  game->currentState.key = newZobristKey;
}