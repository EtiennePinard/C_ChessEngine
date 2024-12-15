#ifndef C485539C_6204_42BC_B1A9_D330E7C34EBE
#define C485539C_6204_42BC_B1A9_D330E7C34EBE

#include "Utils.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Represents a character buffer
 * 
 */
typedef char String[BUFFER_SIZE];

/**
 * @brief Converts the charArray into a string and puts it into resultingString
 * 
 * @param resultingString The string containing the converted charArray
 * @param charArray The character array to convert into a string
 * @return true If the character array can be converted into a string
 * @return false If the character array cannot be converted into a string
 */
bool string_fromCharArray(String *resultingString, const char *charArray);

/**
 * @brief Returns the length of the specified string
 * 
 * @param string The string to get the length of
 * @return size_t The length of the string
 */
size_t string_length(const String string);

/**
 * @brief Returns the element at the index specified
 * 
 * @param string The string to get the element from
 * @param index The index of the element
 * @return char The character at the index. Returns -1 if the index is bigger than the length of the string
 */
char string_characterAt(const String string, size_t index);

/**
 * @brief Inserts the specified character in the given string at the given index
 * 
 * @param string The string to insert the character into
 * @param index The index to insert the character at
 * @param characterToInsert The character to insert
 * @return true If the index is between 0 and string_length(string) - 1
 * @return false If the index is out the bounds of the string
 */
bool string_inserCharacterAt(String string, size_t index, char characterToInsert);

/**
 * @brief Check a string and a charArray for equality
 * 
 * @param string The string to compare the charArray with
 * @param charArray The charArray to compare the string with
 * @return true If the string and charArray are equal
 * @return false If the string and charArray are not equal
 */
bool string_equalsCharArray(String string, char* charArray);

/**
 * @brief Copies the next space token in str to bufferToStoreToken starting at indexToStartAt and 
 * returns the index of the first space ' ' character from the indexToStartAt.
 * If there is no space from the indexToStartAt then it returns the length of the string minus one
 * If indexToStartAt is out the bounds of the string then it returns indexToStartAt
 * Note: This could also happens string_characterAt(string, indexToStartAt) is a space or if 
 * 
 * @param str The string to extract the first word from
 * @param bufferToStoreToken The buffer to store the next space token in
 * @param indexToStartAt The index to start at
 * @return size_t The index of the first space from indexToStartAt
 */
size_t string_nextSpaceTokenStartingAtIndex(const String string, size_t indexToStartAt, String bufferToStoreToken);

/**
 * @brief Converts all the characters in the string to lowercase
 * 
 * @param string The string to convert to lowercase
 */
void string_toLower(String string);

#endif /* C485539C_6204_42BC_B1A9_D330E7C34EBE */
