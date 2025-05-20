#ifndef B646532A_33A4_4979_BB98_A5A4DFCEA463
#define B646532A_33A4_4979_BB98_A5A4DFCEA463

#include "../utils/Constants.h"
#include "../state/Move.h"

/**
 * @brief Heuristically orders the moves to search 'good moves' first
 *
 * @param moves The moves to order
 * @param numMoves The number of moves
 * @param importantMove The hash move or the PV move depending on the context
 * @param board The board state that the moves originate from
 */
void MoveOrdering_orderMoves(Move moves[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES], int numMoves, Move importantMove, const Board currentBoard);

#endif /* B646532A_33A4_4979_BB98_A5A4DFCEA463 */
