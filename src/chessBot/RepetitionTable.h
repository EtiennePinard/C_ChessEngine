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
 * @brief Set the current index as the root position. Call this method before 
 * you start a search, so that the keys which are added to the array can be 
 * discarded after the search is done.
 * 
 */
void RepetitionTable_setCurrentIndexAsRootPosition();

/**
 * @brief Sets the curent index of the table to the root position index
 */
void RepetitionTable_returnToRootPosition();

/**
 * @brief Returns true if the ZobristKey is already contained twice in the table
 * 
 * @param key The key to check if it is contained twice
 * @return true If the key is contained twice in the table
 * @return false If the key is not contained twice in the table
 */
bool RepetitionTable_isKeyContainedTwiceInTable(ZobristKey key);

#endif /* B4E2BCF1_3C75_4170_80B0_A249ED8467D1 */
