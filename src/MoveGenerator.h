#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stddef.h>
#include <stdbool.h>
#include "state/GameState.h"
#include "state/Move.h"

/**
 * Returns true if the friendly king (for the current turn) is in check
 * Only valid after getValidMoves is called
*/
bool isKingInCheck();
/**
 * Returns true if the friendly king (for the current turn) is in double check
 * Only valid after getValidMoves is called
*/
bool isKingInDoubleCheck();

/**
 * Computes the valid moves in a given position and stores the moves in the result array
 * and the amount of moves in the numMoves pointer
*/
void getValidMoves(Move result[256], int* numMoves, ChessPosition game);

#endif