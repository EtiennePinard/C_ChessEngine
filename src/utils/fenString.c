#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "FenString.h"
#include "../state/ZobristKey.h"

static int getCastlingPermFromFenString(const char *castlingPerm) {
  int result = 0;

  size_t lengthOfCasltingPerm = strlen(castlingPerm);
  if (lengthOfCasltingPerm == 0) { return -1; }
  if (castlingPerm[0] == '-') { return 0; }
  if (lengthOfCasltingPerm > 4) { return -1; }

  // We only take characters 'K' = 75, 'Q' = 81, 'k' = 107 and 'q' = 113
  // The shift amounts are:  'K' -> 3, 'Q' -> 2, 'k' -> 1 and 'q' -> 0
  // The hash function is f(c) = (113 - c) / 6
  // We get these results
  // f('K') = 6, f('Q') = 5, f('k') = 1, f('q') = 0
  // This justifies the weird shape of shiftAmounts
  int shiftAmounts[7] = {0, 1, 0, 0, 0, 2, 3};
  for (size_t index = 0; index < lengthOfCasltingPerm; index++) {
    int hashedCharacter = (113 - castlingPerm[index]) / 6;
    result |= (1 << shiftAmounts[hashedCharacter]);
  }
  return result;
}

static bool setBoardArrayFromFenString(const char *fenBoard, Piece board[BOARD_SIZE]) {
  // The minimum length for a board string is 15 with this string "8/8/8/8/8/8/8/8"
  size_t lengthOfBoardFen = strlen(fenBoard);
  if (lengthOfBoardFen < 15) { return false; }

  int index = 0;
  for (size_t str_index = 0; str_index < lengthOfBoardFen; str_index++) {
    char fenChar = fenBoard[str_index];

    switch (fenChar) {
    // Saw this type of trick in https://youtu.be/zGWj7Qo_POY?si=ie9cEXp5p_59KLfU&t=799
    case '8': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '7': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '6': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '5': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '4': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '3': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '2': board[index++] = NOPIECE; __attribute__ ((fallthrough));
    case '1': board[index++] = NOPIECE;
      break;

    case 'P':
      board[index++] = makePiece(WHITE, PAWN);
      break;
    case 'N':
      board[index++] = makePiece(WHITE, KNIGHT);
      break;
    case 'B':
      board[index++] = makePiece(WHITE, BISHOP);
      break;
    case 'R':
      board[index++] = makePiece(WHITE, ROOK);
      break;
    case 'Q':
      board[index++] = makePiece(WHITE, QUEEN);
      break;
    case 'K':
      board[index++] = makePiece(WHITE, KING);
      break;
    case 'p':
      board[index++] = makePiece(BLACK, PAWN);
      break;
    case 'n':
      board[index++] = makePiece(BLACK, KNIGHT);
      break;
    case 'b':
      board[index++] = makePiece(BLACK, BISHOP);
      break;
    case 'r':
      board[index++] = makePiece(BLACK, ROOK);
      break;
    case 'q':
      board[index++] = makePiece(BLACK, QUEEN);
      break;
    case 'k':
      board[index++] = makePiece(BLACK, KING);
      break;
    default:
      // Note: do not increment index on '/' character
      break;
    }
  }

  return index == 64;
}

static PieceCharacteristics getColorToGo(const char *fenColor) {
  if (strlen(fenColor) != 1) { return false; }
  if (fenColor[0] == 'w') {
    return WHITE;
  } else if (fenColor[0] == 'b') {
    return BLACK;
  } else {
    return -1;
  }
}

static inline size_t _getNextWord(const char *fen, size_t spaceIndex, char *splitData) {
  size_t newSpaceIndex = string_nextSpaceCharacterFromIndex(fen, spaceIndex);
  memcpy(splitData, fen + spaceIndex, newSpaceIndex - spaceIndex);
  splitData[newSpaceIndex - spaceIndex] = '\0';
  return newSpaceIndex;
}

bool setChessPositionFromFenString(const char *fen, ChessPosition *position) {
  if (position == NULL || fen == NULL) { return false; }

  char splitData[MAX_FEN_STRING_SIZE] = { 0 };

  size_t spaceIndex = _getNextWord(fen, 0, splitData) + 1;
  Piece boardArray[BOARD_SIZE] = { 0 };
  if (!setBoardArrayFromFenString(splitData, boardArray)) { return false; } 
  Board board = { 0 };
  fromArray(&board, boardArray);
  position->board = board;

  spaceIndex = _getNextWord(fen, spaceIndex, splitData) + 1;
  position->colorToGo = getColorToGo(splitData);
  if ((int) position->colorToGo == -1) { return false; }

  spaceIndex = _getNextWord(fen, spaceIndex, splitData) + 1;
  position->castlingPerm = getCastlingPermFromFenString(splitData);
  if (position->castlingPerm == -1){ return false; }

  spaceIndex = _getNextWord(fen, spaceIndex, splitData) + 1;
  position->enPassantTargetSquare = string_algebraicToIndex(splitData);
  if (position->enPassantTargetSquare == -1) { return false; }

  spaceIndex = _getNextWord(fen, spaceIndex, splitData) + 1;
  position->turnsForFiftyRule = string_parseNumber(splitData);
  if (position->turnsForFiftyRule == -1) { return false; }

  _getNextWord(fen, spaceIndex, splitData);
  position->nbMoves = string_parseNumber(splitData);
  if (position->turnsForFiftyRule == -1) { return false; }

  calculateZobristKey(position);

  return true;
}