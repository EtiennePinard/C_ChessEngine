#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stddef.h>
#include "state/GameState.h"
#include "state/Move.h"

/**
 * Tells which squares are attacked by ennemy pieces for a turn
 * Only valid after getValidMoves is called
*/
extern bool attackedSquares[BOARD_SIZE];
/**
 * If the friendly king (for the current turn) is in check
 * Only valid after getValidMoves is called
*/
extern bool inCheck;
/**
 * If the friendly king (for the current turn) is in double check
 * Only valid after getValidMoves is called
*/
extern bool inDoubleCheck;

/**
 * Computes the valid moves in a given position and stores the moves in the result array
 * and the amount of moves in the numMoves pointer
*/
void getValidMoves(Move result[256], int* numMoves, ChessGame* game);

#endif