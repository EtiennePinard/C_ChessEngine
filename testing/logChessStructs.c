#include "LogChessStructs.h"
#include <ctype.h>

void printMove(Move move) {
    printf("Move: [From: %d, To: %d, Flag: %d]\n", 
      fromSquare(move),
      toSquare(move),
      flagFromMove(move)
    );
}

void printMoveToAlgebraic(Move move) {
  int from = fromSquare(move);
  int to = toSquare(move);
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
      fromSquare(move),
      toSquare(move),
      flagFromMove(move)
    );
}

void writeMoveToAlgebraicToFile(Move move, FILE *file) {
  int from = fromSquare(move);
  int to = toSquare(move);
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

    if ((index % 8) == 0) {
      printf("%d ", 8 - index / 8);
    }

    Piece pieceAtPosition = pieceAtIndex(board, index);
    printf("[%c]", pieceToFenChar(pieceAtPosition));
    printf((index + 1) % 8 == 0 ? "\n" : " ");
  }

  printf("  ");
  for (int i = 0; i < BOARD_LENGTH; i++) {
    printf(" %c  ", 'a' + i);
  }
  printf("\n");
}

void writeBoardToFile(Board board, FILE *file) {
    for (int i = 0; i < BOARD_SIZE; i++) {
    Piece pieceAtPosition = pieceAtIndex(board, i);
    fprintf(file, "[ %c ]", pieceToFenChar(pieceAtPosition));
    if ((i + 1) % 8 == 0 && i != 63) fprintf(file, "\n");
  }
}

void printPosition(ChessPosition position, int score, Move bestMove, float stockfishScore, Move stockfishMove) {
  for (int index = 0; index < BOARD_SIZE; index++) {
    if ((index % 8) == 0) {
      printf("%d ", 8 - index / 8);
    }
    Piece pieceAtPosition = pieceAtIndex(position.board, index);
    printf("[%c]", pieceToFenChar(pieceAtPosition));

    if ((index + 1) % 8 == 0) {
      
      // I can add more cases if I want to display more information
      switch ((index + 1) / 8) {
      case 1:
        printf(" %s to go", position.colorToGo == WHITE ? "white" : "black");
        break;
      case 2:
        printf(" evaluation: %d", score);
        break;
      case 3:
        printf(" best move: [From: %d, To: %d, Flag: %d] (",
               fromSquare(bestMove),
               toSquare(bestMove),
               flagFromMove(bestMove));
        printMoveToAlgebraic(bestMove);
        printf(")");
        break;
      case 4:
        break; // Empty line for more redeability
      case 5:
        printf(" Stockfish eval: %.2f", stockfishScore);
        break;
      case 6:
        printf(" Stockfish best move: [From: %d, To: %d, Flag: %d] (",
               fromSquare(stockfishMove),
               toSquare(stockfishMove),
               flagFromMove(stockfishMove));
        printMoveToAlgebraic(stockfishMove);
        printf(")");
        break;
      default:
        break;
      }

      printf("\n");
    } else {
      printf(" ");
    }
  }
  printf("  ");
  for (int i = 0; i < BOARD_LENGTH; i++) {
    printf(" %c  ", 'a' + i);
  }
  printf("\n");
}

void printBitBoard(u64 bitboard) {
    for (int i = 0; i < 64; i++) {
        printf("%ld", (bitboard >> i) & 1);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
}

void printBin(const u64 num) {
    printf("0b");
    for (int i = 63; i >= 0; i--) {
        printf("%lu", (num >> i) & 1);
    }
    printf("\n");
}
