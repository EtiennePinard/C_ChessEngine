#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#define BOARD_SIZE 64
#define MAX_LEGAL_MOVES 218

#include <stddef.h>

/**
* The structure is copied from Sebastian Lague chess program
* A move is a 16 bit number
* bit 0-5: from square (0 to 63)
* bit 6-11: to square (0 to 63)
* bit 12-15: flag
*/
typedef unsigned short Move;

#define fromSquare(move) (move & 0b111111)
#define targetSquare(move) ((move >> 6) & 0b111111)
#define flag(move) (move >> 12)

#define LAST_MOVE_ARRAY_ELEMENT 0 // This would be an invalid, as the from square from equal the to square, so we use it to encode the length

int nbMoves(Move moves[LAST_MOVE_ARRAY_ELEMENT + 1]);

typedef struct GameState {
    int* boardArray;
    int colorToGo, 
        castlingPerm, // The first bit is for white king side, second bit is for white queen side and pattern continues but for black
        enPassantTargetSquare, 
        turnsForFiftyRule, 
        nbMoves;
} GameState;

/** 
 * Dynamic array of GameState. Use the da_append macro to append GameState
 */
typedef struct gameStates {
    GameState* items;
    size_t count;
    size_t capacity;
} GameStates;


// 4 bit number, so can have total of 16 flags
enum Flag {
    NOFlAG,
    EN_PASSANT,
    DOUBLE_PAWN_PUSH,
    KING_SIDE_CASTLING,
    QUEEN_SIDE_CASTLING,
    PROMOTE_TO_QUEEN,
    PROMOTE_TO_KNIGHT,
    PROMOTE_TO_ROOK,
    PROMOTE_TO_BISHOP,
    STALEMATE,
    CHECKMATE,
    DRAW // For losing in repetition
};

// First 2 bits are for the color
// Last 3 bits are for the type
enum PIECE {
    NONE   = 0, 
    KING   = 1, 
    QUEEN  = 2, 
    KNIGHT = 3, 
    BISHOP = 4, 
    ROOK   = 5, 
    PAWN   = 6, 
            
    WHITE  = 8, 
    BLACK  = 16
};

#define pieceColorBitMask 0b11000
#define pieceTypeBitMask 0b111

// Copy and pasted from https://github.com/tsoding/ded/blob/master/src/common.h#L34
#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

/**
 * Returns the valid moves in a given position
 * The results array is assumed to be 0 initialized
*/
void getValidMoves(Move* results, const GameState currentState, const GameStates previousStates);

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

#endif