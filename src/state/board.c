#include "Board.h"

Piece pieceAtIndex(Board board, int index) {
    return board.boardArray[index];
}

void fromArray(Board* result, Piece array[BOARD_SIZE]) {
    result->boardArray = array;
}