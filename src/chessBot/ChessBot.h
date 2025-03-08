#ifndef A674C591_4B1F_47E1_8F49_FF09345BAD1D
#define A674C591_4B1F_47E1_8F49_FF09345BAD1D

#include "../state/Move.h"
#include "../state/GameState.h"

// Move from square 0 to square 0 with NOFLAG
#define BOT_ERROR NULL_MOVE

/**
* @brief Time control for a chess game in milliseconds. 
* With this time control, the longest game possible will be 2^32 milliseconds, which is
* 49 days 17 hours 2 minutes and 47.3 seconds.
* This should be plenty of time to play a pretty long game.
*/
typedef u32 TimeControl_MS;

/**
 * @brief Sets the position that the bot will analyze.
 * Call this function before any call to Bot_think or Bot_staticEvaluation.  
 * 
 * @param position The position that the bot will analyze
 */
void Bot_provideGameStateForBot(ChessPosition* position);

/**
 * @brief Computes the best move in the position given in the Bot_provideGameStateForBot function
 * 
 * @param whiteRemainingTime The time that white has remaining in milliseconds 
 * @param blackRemainingTime The time that black has remaining in milliseconds
 * @return Move The best move in this position according to the bot or BOT_ERROR if there was an error
 */
Move Bot_think(TimeControl_MS whiteRemainingTime, TimeControl_MS blackRemainingTime);

/**
 * @brief Returns a score that was calculated by statically analyzing the position.
 * Static analysis means only looking at information currently available, not considering 
 * potential good moves in the score.
*/
int Bot_staticEvaluation();

#ifdef DEBUG
    extern bool useTranspositionTable;
    extern int maximumDepth;
#endif

#endif /* A674C591_4B1F_47E1_8F49_FF09345BAD1D */
