#ifndef FC7C9618_1157_4392_921D_5EA7262D829E
#define FC7C9618_1157_4392_921D_5EA7262D829E

#include <stdbool.h>

#include "../utils/Types.h"
#include "../utils/Constants.h"
#include "../state/Piece.h"

typedef int GamePhase;
#define MIDGAME ((GamePhase) 0)
#define ENDGAME ((GamePhase) 1)
#define NB_PHASE ((GamePhase) 2)

// This packs the midgame and endgame into one integer
// This packing allows us to operate on both of them simultaneously
#define S(midGame, endGame) (int) (( ((u16) (endGame)) << 16) | ((u16) (midGame)) )

#define mg_value(score) ((i16) (score & 0xFFFF))

#define eg_value(score) ((i16) (((u32) (score + 0x8000)) >> 16))
// The piece square is gotten from https://github.com/official-stockfish/Stockfish/blob/sf_14/src/psqt.cpp
extern int pieceSquareTable[MAX_PIECE_INDEX - 9 + 1][BOARD_SIZE];

/**
 * @brief Initializes the piece square table
 * 
 * @return true If the piece square table were initialize properly
 * @return false If the piece square table could not be initialize properly
 */
bool PieceSquareTable_init();

#endif /* FC7C9618_1157_4392_921D_5EA7262D829E */
