#ifndef DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
#define DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
// From https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

#include <stdbool.h>

#include "../state/Move.h"
#include "../state/ZobristKey.h"

typedef struct TranspositionTable {
    ZobristKey key;
    char depth;
    Move move;
    int score;
} TranspositionTable;

#define TRANSPOSITION_TABLE_SIZE_IN_MB (1)
 
/**
 * @brief Initialize the transposition table
 * 
 * @return true If the transposition table was correctly initialize
 * @return false If the transposition table could not be correctly initialize 
 */
bool TTable_init();

/**
 * @brief Frees the transposition table memory
 * 
 */
void TTable_terminate();

/**
 * @brief Clears the transposition table
 * 
 */
void TTable_clear();

/**
 * @brief Get the Stored Move object
 * 
 * @param key 
 * @return Move The move from the key or LOOKUP_FAILED 
 */
Move TTable_getMoveFromKey(ZobristKey key);

#endif /* DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B */
