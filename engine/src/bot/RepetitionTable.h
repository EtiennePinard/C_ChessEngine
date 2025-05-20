#ifndef B4E2BCF1_3C75_4170_80B0_A249ED8467D1
#define B4E2BCF1_3C75_4170_80B0_A249ED8467D1

#include <stdbool.h>

#include "../state/ZobristKey.h"

/**
 * @brief Stores the key in the repetition table
 * 
 * @param key The key to store in the repetition table
 * @return true If the key could be store in the table
 * @return false If the key could not be store in the table
 */
bool RepetitionTable_storeKey(ZobristKey key);

/**
 * @brief Removes the last element stored in the repetition table.
 * 
 */
void RepetitionTable_pop();

/**
 * @brief Removes all elements from the repetition table
 * 
 */
void RepetitionTable_clear();

/**
 * @brief Returns true if the ZobristKey is already contained twice in the table
 * 
 * @param key The key to check if it is contained twice
 * @return true If the key is contained twice in the table
 * @return false If the key is not contained twice in the table
 */
bool RepetitionTable_isKeyContainedTwiceInTable(ZobristKey key);

#endif /* B4E2BCF1_3C75_4170_80B0_A249ED8467D1 */
