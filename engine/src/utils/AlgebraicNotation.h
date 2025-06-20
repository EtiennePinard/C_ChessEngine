#ifndef C429AE2B_F4E1_436C_8C95_16BF8E59F724
#define C429AE2B_F4E1_436C_8C95_16BF8E59F724

#include "../state/ChessPosition.h"
#include "../state/Move.h"

/**
 * @brief Converts a move into the Standard algebraic notation. 
 * Note: The buffer needs to have atleast 10 bytes that can be
 * written to.
 * 
 * @param position The position the move will be made in. Please note 
 * that a move will be made in this position which means that the board
 * parameter will be modified. If you wish to not modify your position you
 * need to clone it before putting it in this function
 * @param move The move to make
 * @param buffer The buffer to write to
 */
void moveToStandardAlgebraic(ChessPosition position, Move move, char* buffer);

#endif /* C429AE2B_F4E1_436C_8C95_16BF8E59F724 */
