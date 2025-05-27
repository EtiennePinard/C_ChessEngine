#ifndef FEN_STRING_H
#define FEN_STRING_H

#include <stdbool.h>

#include "../state/ChessPosition.h"

#include "CharBuffer.h"

/**
 * @brief Copies the fen string and then sets ups the chess position from the copied fen string.
 * This function is useful when you have a string constants that you want to convert to a chess position.
 * 
 * @param fen The string containing the fen data which will be copied
 * @param position The ChessPosition to put the fen string into
 * @return true If the fen string is valid and position is not NULL
 * @return false If the fen string is invalid or position is NULL
 */
bool FenString_setChessPositionFromCopiedFenString(const char *fen, ChessPosition *position);

/**
 * @brief Sets ups the chess position from a fen string.
 * 
 * @param fen The string containing the fen data
 * @param position The ChessPosition to put the fen string into
 * @return true If the fen string is valid and position is not NULL
 * @return false If the fen string is invalid or position is NULL
 */
bool FenString_setChessPositionFromFenString(char *fen, ChessPosition* position);

/**
 * @brief Set ups the chess position from a tokenized version of a fen string
 * 
 * @param fenTokenized The tokenized version of a fen string
 * @param position The ChessPosition to put the fen string into
 * @return true If the fen string is valid and position is not NULL
 * @return false If the fen string is invalid or position is NULL
 */
bool FenString_setChessPositionFromTokens(Tokens *fenTokenized, ChessPosition *position);

/**
 * @brief Converts a chess position to a fen string.
 * 
 * @param position The position to convert to a fen string
 * @param fen The resulting char array that the fen string will be put in.
 * It is assumed that this array is big enough to contain the fen string.
 * To guarantee that its size is sufficient just use the MAX_FEN_STRING_SIZE 
 * constants when declaring the array.
 * @return int The actual length of the resulting string 
 */
int FenString_chessPositionToFenString(ChessPosition position, char fen[MAX_FEN_STRING_SIZE]);

#endif