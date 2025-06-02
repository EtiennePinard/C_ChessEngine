#ifndef DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
#define DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B
// From https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

#include <stdbool.h>

#include "../state/Move.h"
#include "../state/ZobristKey.h"

#define LOOKUP_FAILED (-1)

// See https://github.com/SebLague/Chess-Coding-Adventure/blob/f346d2d2ee92422614de063724796537337d702b/Chess-Coding-Adventure/src/Core/Search/TranspositionTable.cs
typedef enum EntryType {
    /**
     * @brief When we computed the evaluation exactly
     */
    EXACT,

    /**
     * @brief When we pruned a branch by the beta parameter
     */
    LOWER_BOUND,

    /**
     * @brief When we pruned a branch by the alpha parameter
     */
    UPPER_BOUND
} EntryType;

typedef struct TTEntry {
    ZobristKey key;
    u8 depth;
    EntryType type;
    int evaluation;

    Move* pvLine;
    int pvLength;
} TTEntry;

#define TRANSPOSITION_TABLE_SIZE_IN_MB (1)

/**
 * @brief Initializes the transposition table
 *
 * @return true If the transposition table was correctly initialized
 * @return false If the transposition table could not be correctly initialize d
 */
bool TranspositionTable_init();

/**
 * @brief Frees the transposition table memory
 *
 */
void TranspositionTable_terminate();

/**
 * @brief Clears the transposition table
 *
 */
void TranspositionTable_clear();

/**
 * @brief Copies the stored pv line from the position with the corresponding Zobrist key
 * to the pvLine array if the entry exist.
 *
 * @param key The key associated with the position to get the pv line in
 * @param pvLine The array to copy the pv line to
 */
void TranspositionTable_getPvLineFromKey(ZobristKey key, Move* pvLine);

/**
 * @brief Get the stored Move object
 *
 * @param key The Zobrist key of the position associated with the search
 * @param depth The depth the search was conducted to
 * @param alpha The alpha parameter of the search
 * @param beta The beta parameter of the search
 * @param plyFromRoot The number of plies (half move) we are from the root (starting search position)
 * @param type Stores the type of the TTEntry if it does not return LOOKUP_FAILED
 * @return Evaluation The evaluation from the key or LOOKUP_FAILED
 */
int TranspositionTable_getEvaluationFromKey(ZobristKey key, int depth, int alpha, int beta, int plyFromRoot, EntryType* type);

/**
 * @brief Records the search entry in the transposition table
 *
 * @param key The Zobrist key of the position associated with the search
 * @param depth The depth the search was conducted to
 * @param type The type of this entry (based on the alpha and beta parameter)
 * @param evaluation The evaluation made in this position
 * @param plyFromRoot The number of plies (half move) we are from the root (starting search position)
 * @param pvLine The pv line in the position
 * @param pvLength The length of pv line from the position
 */
void TranspositionTable_recordEntry(ZobristKey key, u8 depth, EntryType type, int evaluation, int plyFromRoot, Move* pvLine, int pvLength);

#endif /* DABFA9B8_518A_4F61_8F43_4EE80C2CAA6B */
