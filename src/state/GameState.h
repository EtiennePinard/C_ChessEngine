#ifndef BDF83061_7504_461E_BCDD_602085692048
#define BDF83061_7504_461E_BCDD_602085692048

#include <stddef.h>
#include "Board.h"

typedef struct {
    Board board;
    int colorToGo, 
        castlingPerm, // The first bit is for white king side, second bit is for white queen side and pattern continues but for black
        enPassantTargetSquare, 
        turnsForFiftyRule, 
        nbMoves;
} GameState;

GameState* createState(
    int *boardArray,
    int colorToGo, 
    int castlingPerm, 
    int enPassantTargetSquare, 
    int turnsForFiftyRule, 
    int nbMoves
);

/**
 * Utility function to make a copy of a GameState
*/
GameState copyState(GameState from);

#endif /* BDF83061_7504_461E_BCDD_602085692048 */
