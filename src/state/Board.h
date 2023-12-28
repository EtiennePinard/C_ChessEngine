#ifndef E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E
#define E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E

#include <stdint.h>
#include "Piece.h"
#include "Move.h"
#include "Constants.h"

// TODO: Put all the bitboards into an array for fast reading (no switch statements)
/**
 * A struct which holds the information of the chess board, so which piece is at which square.
 * Use the pieceAtIndex function to extract that information
*/
typedef struct Board {
    /* Order of the bitBoard in the array 
    uint64_t Kbb;
    uint64_t Qbb;
    uint64_t Nbb;
    uint64_t Bbb;
    uint64_t Rbb;
    uint64_t Pbb;

    uint64_t kbb;
    uint64_t qbb;
    uint64_t nbb;
    uint64_t bbb;
    uint64_t rbb;
    uint64_t pbb;
    */
   uint64_t bitboards[14];
   // I put 14 total bitboards, even though we only need 12, so that I can assign each piece to a index quickly.
   // The formula for assigning a piece to an index in the array (the hashing function) is piece - 9
   // This works well for WHITE pieces, as they go from 0 to 5
   // However, the BLACK pieces go from 8 to 13
   // Thus index 6 and 7 are invalid in the array
} Board;

/**
 * Returns the piece at a specific index
*/
Piece pieceAtIndex(Board board, int index);

/**
 * Will toggle the bits at a specific index for a piece to the opposite state.
 * If you pass in the NONE piece (0) it will lead to undefined behaviour
*/
void togglePieceAtIndex(Board* board, int index, Piece piece);

void handleMove(Board* board, int from, int to);

/**
 * Converts and puts the information in the piece's `array` to the `result` board.
 * The index of a piece in the array represents its position on the board. 
 * 0 is the top left corner, and 63 is the bottom right corner.
*/
void fromArray(Board* result, Piece array[BOARD_SIZE]);

#endif /* E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E */
