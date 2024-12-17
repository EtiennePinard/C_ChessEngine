#ifndef C485539C_6204_42BC_B1A9_D330E7C34EBE
#define C485539C_6204_42BC_B1A9_D330E7C34EBE

#include "Utils.h"

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Check a two character buffer for equality
 * 
 * @param string1 First character buffer
 * @param string2 Second character buffer
 * @return true If the two character buffers are equal
 * @return false If the two character buffers are not equal
 */
bool string_compareStrings(const char *string1, const char* string2);

/**
 * @brief Returns the index of the first space ' ' character from the indexToStartAt.
 * If there is no space from the indexToStartAt then it returns the length of the string minus one
 * If indexToStartAt is out the bounds of the string then it returns indexToStartAt
 * 
 * @param str The string to extract the first word from
 * @param indexToStartAt The index to start at
 * @return size_t The index of the first space from indexToStartAt
 */
size_t string_nextSpaceCharacterFromIndex(const char *string, size_t indexToStartAt);

/**
 * @brief Converts all the characters in the string to lowercase
 * 
 * @param string The string to convert to lowercase
 */
void string_toLower(char *string);

/**
 * @brief Parses a number from a string object.
 * The format of the number is '{digit_1}...{digit_n}', where digit is a digit
 * There is no minus sign, plus sign or white space allowed
 * 
 * @param string The number in string form to parse
 * @return int The parsed number, or -1 if the string contained invalid characters
 */
int string_parseNumber(const char *string);

/**
 * @brief Converts a square in algebraic notation to a square index used in this program. Returns 0 if square == "-"
 * 
 * @param algebraic The square in algebraic notation
 * @return int The square index of the square in algebraic notation, or -1 if the square char is not well formatted
 */
int string_algebraicToIndex(const char *algebraic);

#endif /* C485539C_6204_42BC_B1A9_D330E7C34EBE */
