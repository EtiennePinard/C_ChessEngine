#ifndef A674C591_4B1F_47E1_8F49_FF09345BAD1D
#define A674C591_4B1F_47E1_8F49_FF09345BAD1D

#include <stdbool.h>

#include "../state/Move.h"
#include "../state/GameState.h"

// Move from square 0 to square 0 with NOFLAG
#define BOT_ERROR NULL_MOVE

// Scores used for mates
#define BOT_INFINITY (2000000)
#define BOT_MINUS_INFINITY (-BOT_INFINITY)

// Default think time
#define BOT_DEFAULT_THINK_TIME_MS (50)

/**
 * @brief Sets the position that the bot will analyze.
 * Call this function before any call to Bot_think or Bot_staticEvaluation.  
 * 
 * @param position The position that the bot will analyze
 */
void Bot_provideGameStateForBot(ChessPosition position);

/**
 * @brief Computes the best move in the position given in the Bot_provideGameStateForBot function
 * 
 * @return Move The best move in this position according to the bot or BOT_ERROR if there was an error
 */
Move Bot_think();

/**
 * @brief Calculates a heuristic for the search time of the bot
 * 
 * @param whiteTime The remaining time for white
 * @param blackTime The remaining time for black
 * @param whiteInc The time increment for white
 * @param blackInc The time increment for black
 * @param isWhiteToMove If it is white to move
 * @return u64 The search time heuristic
 */
u64 Bot_calculateThinkTime(u64 whiteTime, u64 blackTime, u64 whiteInc, u64 blackInc, bool isWhiteToMove);

/**
 * @brief Variable used to stop the search.
 * The volatile label is because this is used
 * by a timer thread which will dictate
 * the search time.
 * 
 */
extern volatile bool endSearch;

#endif /* A674C591_4B1F_47E1_8F49_FF09345BAD1D */
