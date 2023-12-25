#include "logChessStructs.h"
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
  printf("%s", algebraic);
}

void printBoard(int board[BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    int pieceAtPosition = board[i];
    printf("[%d%s]", pieceAtPosition, pieceAtPosition / 10 == 0 ? " " : "");
    printf((i + 1) % 8 == 0 ? "\n" : " ");
  }
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

char pieceToFenChar(int piece) {
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

void writeBoardToFile(int board[64], FILE *file) {
    for (int i = 0; i < BOARD_SIZE; i++) {
    int pieceAtPosition = board[i];
    fprintf(file, "[ %c ]", pieceToFenChar(pieceAtPosition));
    if ((i + 1) % 8 == 0 && i != 63) fprintf(file, "\n");
  }
}