#ifndef F5D96C5E_E343_4A14_B3F3_5D269E6250AC
#define F5D96C5E_E343_4A14_B3F3_5D269E6250AC

// From https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm

#include "../../src/state/ZobristKey.h"

/*
Define DEBUG to debug Zobrist key collisions

#define DEBUG
*/

// Here we cannot define lookup failed as a negative number because perft is unsigned
// So we define it as 0. We should never see a perft of 0 in the perft function because we 
// exit early when that happens
#define LOOKUP_FAILED (0)

/**
 * @brief A perft transposition table entry
 * 
 * @param key The Zobrist key of the position associated with the perft value 
 * @param depth The depth the perft value was calculated at
 * @param perft The perft value calculated
 * @param chessPosition If DEBUG is defined, then we store the chess position to look for 
 * collisions
 */
typedef struct PerftTranspositionTable {
    #ifdef DEBUG
    ChessPosition chessPosition;
    #endif
    ZobristKey key;
    u8 depth;
    u64 perft;
} PerftTranspositionTable;

#define TRANSPOSITION_TABLE_SIZE_IN_MB (32)
 
/**
 * @brief Initialize the transposition table
 * 
 * @return true If the transposition table was correctly initialize
 * @return false If the transposition table could not be correctly initialize 
 */
bool PerftTranspositionTable_init();

/**
 * @brief Frees the transposition table memory
 * 
 */
void PerftTranspositionTable_terminate();

/**
 * @brief Clears the transposition table
 * 
 */
void PerftTranspositionTable_clear();

#ifdef DEBUG
u64 PerftTranspositionTable_getPerftFromKey(ZobristKey key, u8 depth, ChessPosition pos);
#else
/**
 * @brief Get the Stored Move object
 * 
 * @param key The zobrist key of the position to get the perft of
 * @param depth The depth the key is stored at
 * @return Move The move from the key or LOOKUP_FAILED 
 */
u64 PerftTranspositionTable_getPerftFromKey(ZobristKey key, u8 depth);
#endif

/**
 * @brief Records the perft entry in the table
 * 
 * @param entry The perft transposition table entry to record
 */
void PerftTranspositionTable_recordPerft(PerftTranspositionTable entry);

#endif /* F5D96C5E_E343_4A14_B3F3_5D269E6250AC */
