#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "Constants.h"
#include "Math.h"
#include "CharBuffer.h"

#define SPACE_CHAR (' ')

bool string_compareStrings(const char* string1, const char* string2) {
    assert(string1 != NULL);
    assert(string2 != NULL);

    if (string1[0] != string2[0]) { return false; }
    return strcmp(string1, string2) == 0;
}

void string_toLower(char* string) {
    assert(string != NULL);

    for (size_t index = 0; index < strlen(string); index++) {
        string[index] = tolower(string[index]);
    }
}
#include <stdio.h>
size_t string_removeUnecessarySpacesAndTabs(char* string) {
    assert(string != NULL);

    size_t lengthOfString = 0;
    while (true) {
        char character = string[lengthOfString];
        if (character == '\t') string[lengthOfString] = SPACE_CHAR;
        if (character == '\0') break;
        lengthOfString++;
    }

    size_t currentIndex = 0;
    // There is always one token, except if the string is empty or only has spaces
    // The empty string and all spaces case is handled at the first if statement
    size_t nbToken = 1;

    for (; string[currentIndex] == SPACE_CHAR && currentIndex < lengthOfString; currentIndex++);

    if (currentIndex == lengthOfString) {
        // The string contains all spaces or is empty 
        string[0] = '\0';
        return 0;
    }

    if (currentIndex != 0) {
        // The string contains leading spaces
        memmove(string, string + currentIndex, lengthOfString - currentIndex + 1); // The +1 is because we want the NULL byte
        lengthOfString -= currentIndex;
    }

    // We are now at a point where we are sure that the string does not start with a space
    currentIndex = 0;
    for (; string[currentIndex] != SPACE_CHAR && currentIndex < lengthOfString; currentIndex++);

    size_t nextSpaceIndex = 0;

    while (currentIndex < lengthOfString) {
        for (nextSpaceIndex = currentIndex + 1; nextSpaceIndex < lengthOfString && string[nextSpaceIndex] == SPACE_CHAR; nextSpaceIndex++);

        if (nextSpaceIndex == lengthOfString) {
            // The string ends with spaces
            string[currentIndex] = '\0';
            break;
        }

        currentIndex++;
        nbToken++;

        if (nextSpaceIndex > currentIndex + 1) {
            // There is more than one space between the non space character, so we need to trim them
            memmove(string + currentIndex, string + nextSpaceIndex, lengthOfString - nextSpaceIndex + 1);
            lengthOfString -= (nextSpaceIndex - currentIndex);
        }
        if (string[currentIndex] == '\n') {
            // We need to put this '\n' to the end of the previous token
            memmove(string + currentIndex - 1, string + currentIndex, lengthOfString - currentIndex + 1);
            lengthOfString--;
            nbToken--;
        }

        for (; string[currentIndex] != SPACE_CHAR && currentIndex < lengthOfString; currentIndex++);
    }
    return nbToken;
}

void string_tokenizeStringBySpace(char* string, Tokens* result) {
    assert(string != NULL);

    // Replacing the space character by '\0' and appending the tokens to tokens
    size_t currentIndex = 0;
    size_t lastSpaceIndex = 0;
    size_t lengthOfString = strlen(string);

    size_t tokenIndex = 0;

    while (currentIndex < lengthOfString) {
        for (; string[currentIndex] != SPACE_CHAR && currentIndex < lengthOfString; currentIndex++);

        // currentIndex now points to a space character
        string[currentIndex++] = '\0';

        result->tokens[tokenIndex++] = string + lastSpaceIndex;

        lastSpaceIndex = currentIndex;
    }
}

int string_parseNumber(const char* num) {
    assert(num != NULL);

    int lengthOfNum = (int)strlen(num);
    if (lengthOfNum == 0) { return -1; }

    int result = 0;

    // Starting from the end
    // Also the index < lengthOfNum condition because of unsigned values wrapping
    for (int index = lengthOfNum - 1; index >= 0; index--) {
        char character = num[index];
        if (character < '0' || character > '9') { return -1; }

        int numToAdd = character - '0';
        for (int pow = index; pow < lengthOfNum - 1; pow++) {
            numToAdd *= 10;
        }

        result += numToAdd;
    }

    return result;
}

int string_algebraicToIndex(const char* algebraic) {
    assert(algebraic != NULL);

    size_t lengthOfSquare = strlen(algebraic);
    if (lengthOfSquare == 0) { return -1; }
    if (algebraic[0] == '-') { return 0; }
    if (lengthOfSquare != 2) { return -1; }

    if (algebraic[0] < 'a' || algebraic[0] > 'h') { return -1; }
    if (algebraic[1] < '1' || algebraic[1] > '9') { return -1; }
    int file = algebraic[0] - 'a';
    int rank = 8 - (algebraic[1] - '0');
    return rank * 8 + file;
}

Move string_longAlgebraicToMove(const char* algebraic) {
    size_t algebraicLength = strlen(algebraic);

    if (algebraicLength < 4) {
        return NULL_MOVE;
    }

    int from = string_algebraicToIndex((char[]) { algebraic[0], algebraic[1], '\0' });
    int to = string_algebraicToIndex((char[]) { algebraic[2], algebraic[3], '\0' });

    if (from == -1 || to == -1) {
        return NULL_MOVE;
    }

    Flag flag = NOFLAG;
    // Checking for promotion flags
    if (algebraicLength >= 5) {
        switch (algebraic[4]) {
        case 'q': flag = PROMOTE_TO_QUEEN; break;
        case 'n': flag = PROMOTE_TO_KNIGHT; break;
        case 'r': flag = PROMOTE_TO_ROOK; break;
        case 'b': flag = PROMOTE_TO_BISHOP; break;
        default: break; // If it matches neither then we simply have no flags
        }
    }
    return Move_makeMove(from, to, flag);
}

int string_moveToLongAlgebraic(Move move, char buffer[6]) {
    assert(buffer != NULL);

    Square from = Move_fromSquare(move);
    buffer[0] = 'a' + file(from);
    buffer[1] = '1' + (7 - rank(from));

    Square to = Move_toSquare(move);
    buffer[2] = 'a' + file(to);
    buffer[3] = '1' + (7 - rank(to));

    Flag flag = Move_flag(move);

    switch (flag) {
    case PROMOTE_TO_BISHOP: buffer[4] = 'b'; break;
    case PROMOTE_TO_KNIGHT: buffer[4] = 'n'; break;
    case PROMOTE_TO_ROOK: buffer[4] = 'r'; break;
    case PROMOTE_TO_QUEEN: buffer[4] = 'q'; break;
    default: buffer[4] = '\0'; return 4;
    }
    buffer[5] = '\0';
    return 5;
}
