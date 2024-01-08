#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include <stddef.h>
#include "Board.h"

typedef struct {
    Board board;
    PieceCharacteristics colorToGo;
    int castlingPerm, // The first bit is for white king side, second bit is for white queen side and pattern continues but for black
        enPassantTargetSquare, 
        turnsForFiftyRule, 
        nbMoves;
} GameState;

/**
 * Returns the number of elements in an array of gamestates.
 * Assumes that the last element of the array is 0
*/
size_t nbGameStatesInArray(GameState* gameStates);

GameState* createState(
    Board board,
    int colorToGo, 
    int castlingPerm, 
    int enPassantTargetSquare, 
    int turnsForFiftyRule, 
    int nbMoves
);

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
