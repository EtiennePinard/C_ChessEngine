#ifndef A674C591_4B1F_47E1_8F49_FF09345BAD1D
#define A674C591_4B1F_47E1_8F49_FF09345BAD1D

#include "../state/Move.h"
#include "../state/GameState.h"

/**
 * Returns the best move in a position (according to the computer)
 * TODO: Implement time control later
*/
Move think(ChessGame* game);

/**
 * Returns a score that was calculated by statically analyzing the position.
 * Static analysis means only looking at information curently available, not considering 
 * potential good moves in the score.
*/
int staticEvaluation(ChessGame* game);

#endif /* A674C591_4B1F_47E1_8F49_FF09345BAD1D */
