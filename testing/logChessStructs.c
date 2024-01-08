#include "LogChessStructs.h"
#include <ctype.h>

void printMove(Move move) {
    printf("Move: [From: %d, To: %d, Flag: %d]\n", 
      move & 0b111111,
      (move >> 6) & 0b111111,
      (move >> 12)
    );
}

void printMoveToAlgebraic(Move move) {
  int from = move & 0b111111;
  int to = (move >> 6) & 0b111111;
  char algebraic[6];
  int row1 = from / 8;
  int col1 = from % 8;
  algebraic[0] = 'a' + col1;
  algebraic[1] = '1' + (7 - row1);
  int row2 = to / 8;
  int col2 = to % 8;
  algebraic[2] = 'a' + col2;
  algebraic[3] = '1' + (7 - row2);
  switch (flagFromMove(move)) {
  case PROMOTE_TO_BISHOP:
    algebraic[4] = 'b';
    algebraic[5] = '\0';
    break;
  case PROMOTE_TO_KNIGHT:
    algebraic[4] = 'n';
    algebraic[5] = '\0';
    break;
  case PROMOTE_TO_ROOK:
    algebraic[4] = 'r';
    algebraic[5] = '\0';
    break;    
  case PROMOTE_TO_QUEEN:
    algebraic[4] = 'q';
    algebraic[5] = '\0';
    break;       
  default:
    algebraic[4] = '\0';
  }
  
  printf("%s", algebraic);
}

void writeMoveToFile(Move move, FILE *file) {
      fprintf(file, "[From: %d, To: %d, Flag: %d], ", 
      move & 0b111111,
      (move >> 6) & 0b111111,
      (move >> 12)
    );
}

void writeMoveToAlgebraicToFile(Move move, FILE *file) {
  int from = move & 0b111111;
  int to = (move >> 6) & 0b111111;
  char algebraic[5];
  int row1 = from / 8;
  int col1 = from % 8;
  algebraic[0] = 'a' + col1;
  algebraic[1] = '1' + (7 - row1);
  int row2 = to / 8;
  int col2 = to % 8;
  algebraic[2] = 'a' + col2;
  algebraic[3] = '1' + (7 - row2);
  algebraic[4] = '\0';
  fprintf(file, "%s", algebraic);
}

char pieceToFenChar(Piece piece) {
  char result;
  switch (pieceType(piece)) {
    case KING:
      result = 'k';
    break;
    case QUEEN:
      result = 'q';  
    break;
    case ROOK: 
      result = 'r';       
    break;
    case BISHOP:
      result = 'b';   
    break;
    case KNIGHT:
      result = 'n';   
    break;
    case PAWN:
      result = 'p';
    break;
    default:
      return ' ';
    break;
  }
  if (pieceColor(piece) == WHITE) {
    result = toupper(result);
  }
  return result;
}

void printBoard(Board board) {
  for (int index = 0; index < BOARD_SIZE; index++) {
    Piece pieceAtPosition = pieceAtIndex(board, index);
    printf("[%c]", pieceToFenChar(pieceAtPosition));
    printf((index + 1) % 8 == 0 ? "\n" : " ");
  }
}

void writeBoardToFile(Board board, FILE *file) {
    for (int i = 0; i < BOARD_SIZE; i++) {
    Piece pieceAtPosition = pieceAtIndex(board, i);
    fprintf(file, "[ %c ]", pieceToFenChar(pieceAtPosition));
    if ((i + 1) % 8 == 0 && i != 63) fprintf(file, "\n");
  }
}