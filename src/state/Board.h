#ifndef E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E
#define E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E

#include "Piece.h"
#include "Constants.h"

// TODO: Implement bitboards
/**
 * A struct which holds the information of the chess board, so which piece is at which square.
 * Use the pieceAtIndex function to extract that information
*/
typedef struct {
    Piece* boardArray;
} Board;

/**
 * Returns the piece at a specific index
*/
Piece pieceAtIndex(Board board, int index);

/**
 * Converts and puts the information in the piece's `array` to the `result` board.
 * The index of a piece in the array represents its position on the board. 
 * 0 is the top left corner, and 63 is the bottom right corner.
*/
void fromArray(Board* result, Piece array[BOARD_SIZE]);

#endif /* E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E */
