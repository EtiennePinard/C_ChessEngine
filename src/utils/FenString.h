#ifndef FEN_STRING_H
#define FEN_STRING_H

#include <stdbool.h>
#include "../state/GameState.h"
#include "CharBuffer.h"

/**
 * @brief Sets ups the chess position from a fen string.
 * 
 * @param fen The string containing the fen data
 * @param position The ChessPosition to put the fen string into
 * @return true If the fen string is valid and position is not NULL
 * @return false If the fen string is invalid or position is NULL
 */
bool FenString_setChessPositionFromFenString(const char *fen, ChessPosition* position);

#endif