#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stddef.h>
#include "state/GameState.h"
#include "state/Move.h"

/**
 * Computes the valid moves in a given position and stores the moves in the result array
 * and the amount of moves in the numMoves pointer
*/
void getValidMoves(Move result[256], int* numMoves, ChessGame* game);

#endif