#ifndef FEN_STRING_H
#define FEN_STRING_H

#include <stdbool.h>
#include "../state/GameState.h"

/**
 * Sets ups the chess position currently from a fen string.
 * Returns true if the fenstring is valid and result != NULL
*/
bool setChessPositionFromFenString(const char* fenString, ChessPosition* result);

#endif