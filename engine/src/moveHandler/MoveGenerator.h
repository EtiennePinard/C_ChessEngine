#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stdbool.h>

#include "../state/ChessPosition.h"
#include "../state/Move.h"

/**
 * @brief Returns true if the friendly king (for the current turn) is in check
 * 
 * IMPORTANT: Will return a correct result only after MoveHandler_getValidMoves is called
 *
 * @return true If the king is in check
 * @return false If the king is not in check
 */
bool MoveHandler_isKingInCheck();

/**
 * @brief Returns true if the friendly king (for the current turn) is in double check
 * 
 * IMPORTANT: Will return a correct result only after MoveHandler_getValidMoves is called
 *
 * @return true If the king is in double check
 * @return false If the king is not in double check
 */
bool MoveHandler_isKingInDoubleCheck();

/**
 * @brief Computes the valid moves in a given position and stores the moves in the result array
 * and the amount of moves in the numMoves pointer.
 *
 * IMPORTANT: The MagicBitBoard_init() and ZobristKey_init() functions needs to be invoked once
 * before calling this function.
 *
 * @param result The valid moves in the position
 * @param numMoves The number of valid moves in this position
 * @param position The position to get the valid moves
 */
void MoveHandler_getValidMoves(Move result[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES], int* numMoves, ChessPosition position);

/**
 * @brief Computes the valid captures in a given position and stores them in the result array
 * and the amount of captures in the numCaptures pointer.
 *
 * IMPORTANT: The MagicBitBoard_init() and ZobristKey_init() functions needs to be invoked once
 * before calling this function.
 *
 * @param result The valid captures in the position
 * @param numMoves The number of valid captures in this position
 * @param position The position to get the valid captures
 */
void MoveHandler_getCaptures(Move result[POWER_OF_TWO_CLOSEST_TO_MAX_LEGAL_MOVES], int* numCaptures, ChessPosition position);

/**
 * @brief Corrects the move flag of move if necessary.
 * 
 * @param position The position the move will be made in
 * @param move The move to make in the position
 * @return Move The move with the correct flags
 */
Move MoveHandler_correctMoveFlag(ChessPosition position, Move move);

#endif
