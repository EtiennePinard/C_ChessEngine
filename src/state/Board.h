#ifndef E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E
#define E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E

#include <stdint.h>
#include "Piece.h"
#include "Move.h"
#include "../utils/Utils.h"

/**
 * A struct which holds the information of the chess board, so which piece is at which square.
 * Use the pieceAtIndex function to extract that information
*/
typedef struct Board {
    /* Order of the bitBoard in the array 

    0 -> White King
    1 -> White Knight
    2 -> White Bishop
    3 -> White Queen
    4 -> White Rook
    5 -> White Pawn

    6 -> UNUSED
    7 -> UNUSED

    8 -> Black King
    9 -> Black Knight 
    10 -> Black Bishop
    11 -> Black Queen
    12 -> Black Rook
    13 -> Black Pawn
    */
   u64 bitboards[14];
   // I put 14 total bitboards, even though we only need 12, so that I can assign each piece to a index quickly.
   // The formula for assigning a piece to an index in the array (the hashing function) is (piece - 9)
   // This works well for WHITE pieces, as (white piece - 9) will yield results from 0 to 5
   // However, (black piece - 9) will yield results from 8 to 13
   // Thus, index 6 and 7 are invalid in the array
} Board;

u64 whitePiecesBitBoard(Board board);
u64 blackPiecesBitBoard(Board board);
u64 allPiecesBitBoard(Board board);

/**
 * Returns the piece at a specific index
*/
Piece pieceAtIndex(Board board, int index);

/**
 * Will toggle the bits at a specific index for a piece to the opposite state.
 * If you pass in the NOPIECE piece (0) it will lead to undefined behaviour
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
