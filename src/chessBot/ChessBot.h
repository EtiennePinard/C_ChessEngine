#ifndef A674C591_4B1F_47E1_8F49_FF09345BAD1D
#define A674C591_4B1F_47E1_8F49_FF09345BAD1D

#include "../state/Move.h"
#include "../state/GameState.h"

// Move from square 0 to square 0 with NOFLAG
#define BOT_ERROR ((Move) 0)

/**
 * Call this function with the ChessGame that you want the bot to analyze
*/
void provideGameStateForBot(ChessGame* game);

/**
 * Returns the best move in a position (according to the computer)
 * Returns BOT_ERROR if there was an error
 * TODO: Implement time control later
*/
Move think();

/**
 * Returns a score that was calculated by statically analyzing the position.
 * Static analysis means only looking at information curently available, not considering 
 * potential good moves in the score.
*/
int staticEvaluation();

#endif /* A674C591_4B1F_47E1_8F49_FF09345BAD1D */
