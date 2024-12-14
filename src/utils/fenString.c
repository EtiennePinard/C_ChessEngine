#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "FenString.h"
#include "../state/ZobristKey.h"

int getCastlingPermFromFenString(char* fen) {
    int result = 0;
    if (strcmp(fen, "-") == 0) return result;
    for (size_t i = 0; i < strlen(fen); i++) {
        char c = fen[i];
        if (c == 'K') {
            result |= (1 << 3);
        } else if (c == 'Q') {
            result |= (1 << 2);
        } else if (c == 'k') {
            result |= (1 << 1);
        } else if (c == 'q') {
            result |= (1 << 0);
        }
    }
    return result;
}

void setBoardArrayFromFenString(char* fen, Piece* board) {
  int row = 0, col = 0;
  for (size_t i = 0; i < strlen(fen); i++) {
    char fenChar = fen[i];
    if (fenChar == '/') {
      row++;
      col = 0;
    } else if (fenChar >= '1' && fenChar <= '8') {
      int emptySquares = fenChar - '0';
      for (int j = 0; j < emptySquares; j++) {
        board[row * 8 + col] = NOPIECE;
        col++;
      }
    } else {
      switch (fenChar) {
        case 'P':
          board[row * 8 + col] = WHITE | PAWN;
          break;
        case 'N':
          board[row * 8 + col] = WHITE | KNIGHT;
          break;
        case 'B':
          board[row * 8 + col] = WHITE | BISHOP;
          break;
        case 'R':
          board[row * 8 + col] = WHITE | ROOK;
          break;
        case 'Q':
          board[row * 8 + col] = WHITE | QUEEN;
          break;
        case 'K':
          board[row * 8 + col] = WHITE | KING;
          break;
        case 'p':
          board[row * 8 + col] = BLACK | PAWN;
          break;
        case 'n':
          board[row * 8 + col] = BLACK | KNIGHT;
          break;
        case 'b':
          board[row * 8 + col] = BLACK | BISHOP;
          break;
        case 'r':
          board[row * 8 + col] = BLACK | ROOK;
          break;
        case 'q':
          board[row * 8 + col] = BLACK | QUEEN;
          break;
        case 'k':
          board[row * 8 + col] = BLACK | KING;
          break;
      }
      col++;
    }
  }
}

int convertSquareToIndex(char* square) {
  u8 lengthOfSquare = strlen(square);
  assert(lengthOfSquare > 0 && "Length of square string is 0");
  if (square[0] == '-') { return 0; }
  assert(strlen(square) >= 2 && "Length of square string is less than 2");
  int file = square[0] - 'a';
  int rank = 8 - (square[1] - '0');
  return rank * 8 + file;
}

bool setChessPositionFromFenString(const char fen[BUFFER_SIZE], ChessPosition* position) {
    if (position == NULL) { return false; }

    u8 fenStringLength = strlen(fen);
    char split[BUFFER_SIZE];
    u8 spaceIndex = nextSpaceTokenStartingAtIndex(fen, 0, split);
    if (spaceIndex == fenStringLength) { return false; }

    Piece boardArray[BOARD_SIZE] = { 0 };
    setBoardArrayFromFenString(split, boardArray);

    Board board = { 0 };
    fromArray(&board, boardArray);
    position->board = board;
    
    spaceIndex = nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
    if (spaceIndex == fenStringLength) { return false; }
    position->colorToGo = (strcmp(split, "w") == 0) ? WHITE : BLACK;

    spaceIndex = nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
    if (spaceIndex == fenStringLength) { return false; }  
    position->castlingPerm = getCastlingPermFromFenString(split);

    spaceIndex = nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
    if (spaceIndex == fenStringLength) { return false; }
    position->enPassantTargetSquare = convertSquareToIndex(split);

    spaceIndex = nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
    if (spaceIndex == fenStringLength) { return false; }
    position->turnsForFiftyRule = (int) strtol(split, NULL, 10);

    spaceIndex = nextSpaceTokenStartingAtIndex(fen, spaceIndex + 1, split);
    // Here it makes sense if the space == fenStringLength since we could be at the end of the string
    position->nbMoves = (int) strtol(split, NULL, 10);

    calculateZobristKey(position);

    return true;
}