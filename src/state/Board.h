#ifndef E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E
#define E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E

#include "Piece.h"
#include "../utils/Utils.h"

typedef u64 BitBoard;

/**
 * @brief A struct which holds the information of the chess board, so which piece is at which square.
 * Use the pieceAtIndex function to extract that information
 */
typedef struct Board {
    /* Order of the bitBoard in the array 

    0 -> White Pawn
    1 -> White Knight
    2 -> White Bishop
    3 -> White Rook
    4 -> White Queen
    5 -> White King

    6 -> UNUSED
    7 -> UNUSED

    8 -> Black Pawn
    9 -> Black Knight 
    10 -> Black Bishop
    11 -> Black Rook
    12 -> Black Queen
    13 -> Black King
    */
   BitBoard bitboards[MAX_PIECE_INDEX - 9 + 1];
   // I put 14 total bitboards, even though we only need 12, so that I can assign each piece to a index quickly.
   // The formula for assigning a piece to an index in the array (the hashing function) is (piece - 9)
   // This works well for WHITE pieces, as (white piece - 9) will yield results from 0 to 5
   // However, (black piece - 9) will yield results from 8 to 13
   // Thus, index 6 and 7 are invalid in the array
} Board;

/**
 * @brief Returns the bit board of a specific piece
 * 
 * @param board The board struct containing all the bit boards
 * @param piece The piece to get the bitboard of
 * @return BitBoard The bitboard of the piece
 */
BitBoard Board_bitBoardForPiece(Board board, Piece piece);

/**
 * @brief Returns the bitboard of all pieces in a specific color.
 * Is equivalent to calling Board_bitBoardForPiece for all pieces
 * in the color and bitwise or all the bit boards together
 * 
 * @param board The board struct containing all the bit boards
 * @param color The color of the pieces to get the bitboard of 
 * @return BitBoard The bitboard of all the white or black pieces
 */
BitBoard Board_specificColorBitBoard(Board board, PieceCharacteristics color);

/**
 * @brief Returns the bitboard of all pieces.
 * Is equivalent to doing Board_bitBoardForPiece(board, WHITE) | Board_bitBoardForPiece(board, BLACK)
 * 
 * @param board 
 * @return BitBoard 
 */
BitBoard Board_allPiecesBitBoard(Board board);

/**
 * @brief Gets the piece at a specific index 
 * Note: This function does not check if the index is valid, so it can
 * lead to undefined behaviour if the index is bigger than 63
 * 
 * @param board The bitboards of all pieces
 * @param index The index of the chess board to get the piece
 * @return Piece The piece at that index
 */
Piece Board_pieceAtIndex(Board board, u8 index);

/**
 * @brief Puts the bits at a specific index of a piece bit board to the opposite state.
 *  Note: This function does not check if the index is valid, so it can
 * lead to undefined behaviour if the index is bigger than 63
 * 
 * @param board The board containing the bitboards
 * @param index The index to toggle the bit on or off
 * @param piece The bitboard to toggle the bits in
 */
void Board_togglePieceAtIndex(Board* board, u8 index, Piece piece);

/**
 * @brief Converts and puts the information in the piece's `array` to the `result` board.
 * The index of a piece in the array represents its position on the board with 
 * 0 being the top left corner, and 63 being the bottom right corner.
 * 
 * @param result The board struct to fill based on the array parameter
 * @param array The array of pieces which represent the chess board
 */
void Board_fromArray(Board* result, Piece array[BOARD_SIZE]);

#endif /* E6F9D86C_5687_4DF1_9FEB_54DDB21FA54E */
