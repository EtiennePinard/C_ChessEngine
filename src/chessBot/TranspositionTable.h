#ifndef DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
#define DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
// From https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

#include <stdbool.h>

#include "../state/Move.h"
#include "../state/ZobristKey.h"

#define LOOKUP_FAILED (-1)

// See https://github.com/SebLague/Chess-Coding-Adventure/blob/f346d2d2ee92422614de063724796537337d702b/Chess-Coding-Adventure/src/Core/Search/TranspositionTable.cs
typedef enum EntryType {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
} EntryType;

typedef struct TranspositionTable {
    ZobristKey key;
    u8 depth;
    EntryType type;
    Move move;
    int evaluation;
} TranspositionTable;

#define B sizeof(TranspositionTable);

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
