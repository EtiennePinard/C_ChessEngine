#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "FenString.h"
#include "../state/ZobristKey.h"

int getCastlingPermFromFenString(String castlingPerm)
{
  int result = 0;

  size_t lengthOfCasltingPerm = string_length(castlingPerm);
  assert(lengthOfCasltingPerm > 0 && "Length of square string is 0");
  if (string_characterAt(castlingPerm, 0) == '-')
  {
    return 0;
  }
  assert(lengthOfCasltingPerm >= 2 && "Length of square string is less than 2");

  for (size_t index = 0; index < lengthOfCasltingPerm; index++)
  {
    char c = string_characterAt(castlingPerm, index);
    if (c == 'K')
    {
      result |= (1 << 3);
    }
    else if (c == 'Q')
    {
      result |= (1 << 2);
    }
    else if (c == 'k')
    {
      result |= (1 << 1);
    }
    else if (c == 'q')
    {
      result |= (1 << 0);
    }
  }
  return result;
}

void setBoardArrayFromFenString(const String fenBoard, Piece board[BOARD_SIZE]) {
  // The minimum length for a board string is 15 with this string "8/8/8/8/8/8/8/8"
  size_t lengthOfBoardFen = string_length(fenBoard);
  assert(lengthOfBoardFen >= 15 && "The minimum length for a board string is 15 with this string \"8/8/8/8/8/8/8/8\"");

  int index = 0;
  for (size_t str_index = 0; str_index < lengthOfBoardFen; str_index++) {
    char fenChar = string_characterAt(fenBoard, str_index);

    switch (fenChar) {
    // Saw this type of trick in https://youtu.be/zGWj7Qo_POY?si=ie9cEXp5p_59KLfU&t=799
    case '8': board[index++] = NOPIECE;
    case '7': board[index++] = NOPIECE;
    case '6': board[index++] = NOPIECE;
    case '5': board[index++] = NOPIECE;
    case '4': board[index++] = NOPIECE;
    case '3': board[index++] = NOPIECE;
    case '2': board[index++] = NOPIECE;
    case '1': board[index++] = NOPIECE;
      break;

    case 'P':
      board[index++] = WHITE | PAWN;
      break;
    case 'N':
      board[index++] = WHITE | KNIGHT;
      break;
    case 'B':
      board[index++] = WHITE | BISHOP;
      break;
    case 'R':
      board[index++] = WHITE | ROOK;
      break;
    case 'Q':
      board[index++] = WHITE | QUEEN;
      break;
    case 'K':
      board[index++] = WHITE | KING;
      break;
    case 'p':
      board[index++] = BLACK | PAWN;
      break;
    case 'n':
      board[index++] = BLACK | KNIGHT;
      break;
    case 'b':
      board[index++] = BLACK | BISHOP;
      break;
    case 'r':
      board[index++] = BLACK | ROOK;
      break;
    case 'q':
      board[index++] = BLACK | QUEEN;
      break;
    case 'k':
      board[index++] = BLACK | KING;
      break;
    default:
      // Note: do not increment index on '/' character
      break;
    }
  }

  assert(index == 64 && "The fen string board is invalid!");
}

int convertSquareToIndex(const String square) {
  size_t lengthOfSquare = string_length(square);
  assert(lengthOfSquare > 0 && "Length of square string is 0");
  if (string_characterAt(square, 0) == '-')
  {
    return 0;
  }
  assert(lengthOfSquare >= 2 && "Length of square string is less than 2");

  int file = string_characterAt(square, 0) - 'a';
  int rank = 8 - (string_characterAt(square, 1) - '0');
  return rank * 8 + file;
}

bool setChessPositionFromFenString(const String fen, ChessPosition *position)
{
  if (position == NULL)
  {
    return false;
  }

  size_t fenStringLength = string_length(fen);
  String split;

  size_t spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, 0, split);
  if (spaceIndex == fenStringLength)
  {
    return false;
  }
  Piece boardArray[BOARD_SIZE] = {0};
  setBoardArrayFromFenString(split, boardArray);
  Board board = {0};
  fromArray(&board, boardArray);
  position->board = board;

  spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
  if (spaceIndex == fenStringLength)
  {
    return false;
  }
  position->colorToGo = (strcmp(split, "w") == 0) ? WHITE : BLACK;

  spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
  if (spaceIndex == fenStringLength)
  {
    return false;
  }
  position->castlingPerm = getCastlingPermFromFenString(split);

  spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
  if (spaceIndex == fenStringLength)
  {
    return false;
  }
  position->enPassantTargetSquare = convertSquareToIndex(split);

  spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
  if (spaceIndex == fenStringLength)
  {
    return false;
  }
  position->turnsForFiftyRule = (int)strtol(split, NULL, 10);

  spaceIndex = string_nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
  // Here it makes sense if the space == fenStringLength since we could be at the end of the string
  position->nbMoves = (int)strtol(split, NULL, 10);

  calculateZobristKey(position);

  return true;
}