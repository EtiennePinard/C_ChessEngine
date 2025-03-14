#ifndef C485539C_6204_42BC_B1A9_D330E7C34EBE
#define C485539C_6204_42BC_B1A9_D330E7C34EBE

#include <stddef.h>
#include <stdbool.h>
#include "../state/Move.h"

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
 * @brief Returns the index of the first space ' ' character in the string.
 * If there is no space then it returns the length of the string.
 * Note: If you need to start an someplace else than the beginning of the string, just
 * do string_nextSpaceCharacterFromIndex(string + offset)
 * 
 * @param str The string to extract the first word from
 * @return size_t The index of the first space from indexToStartAt
 */
size_t string_nextSpaceCharacterFromIndex(const char *string);

/**
 * @brief Converts all the characters in the string to lowercase
 * 
 * @param string The string to convert to lowercase
 */
void string_toLower(char *string);

/**
 * @brief Trims contiguous spaces, meaning a double space becomes a single space and so on. 
 * Will also remove leading and ending spaces.
 * 
 * @param string The string to remove the unecessary spaces from
 */
void string_removeUnecessarySpaces(char *string);

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
 * @brief Converts a square in algebraic notation to a square index used in this program. Returns 0 if square == "-".
 * 
 * A string in algebraic notation is of the type: "c_1c_2" with c_1 begin a character from 'a' to 'h' and c_2 begin a digit from 1 to 9.
 * 
 * In this program the indices are flipped from the conventional of assigning indices to a chessboard, meaning that 
 * the chess board is:
 * 
 *   +----+----+----+----+----+----+----+----+ 
 *   |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 
 *   +----+----+----+----+----+----+----+----+
 *   |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |  7
 *   +----+----+----+----+----+----+----+----+
 *   | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |  6
 *   +----+----+----+----+----+----+----+----+
 *   | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |  5
 *   +----+----+----+----+----+----+----+----+
 *   | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |  4
 *   +----+----+----+----+----+----+----+----+
 *   | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |  3
 *   +----+----+----+----+----+----+----+----+
 *   | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |  2
 *   +----+----+----+----+----+----+----+----+
 *   | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |  1
 *   +----+----+----+----+----+----+----+----+
 *      a    b    c    d    e    f    g    h  
 * 
 * @param algebraic The square in algebraic notation
 * @return int The square index of the square in algebraic notation, or -1 if the square char is not well formatted
 */
int string_algebraicToIndex(const char *algebraic);

/**
 * @brief Converts a move to its long algebraic notation. Puts the data in the buffer.
 * Note: The buffer is assumed to have atleast 6 bytes of space.
 * Note: This function assumes that move represents a valid move
 * 
 * @param move The move to convert to its long algebraic notation
 * @param buffer The buffer to store the notation data into
 */
void string_moveToLongAlgebraic(Move move, char buffer[6]);

#endif /* C485539C_6204_42BC_B1A9_D330E7C34EBE */
