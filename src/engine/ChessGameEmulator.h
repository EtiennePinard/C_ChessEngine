#ifndef CEE33778_9653_4427_BB21_5DA3340DEB43
#define CEE33778_9653_4427_BB21_5DA3340DEB43

#include "../state/GameState.h"
#include "../state/Move.h"

/**
 * @brief Makes the move and updates the ChessPosition object. 
 * Note: The move is assumed to be legal
 * 
 * @param move The move to make
 * @param position The ChessPosition object to make the move on 
 * @param storePositionInRepetitionTable If you should store the position in the repetition table
 */
void Engine_playMove(Move move, ChessPosition* position, bool storePositionInRepetitionTable);

#endif /* CEE33778_9653_4427_BB21_5DA3340DEB43 */
