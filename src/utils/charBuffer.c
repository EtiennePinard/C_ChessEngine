#include <string.h>
#include <ctype.h>

#include "CharBuffer.h"

static inline size_t _string_capacity(const String string) {
    return BUFFER_SIZE;
}

bool string_fromCharArray(String *resultingString, const char *charArray) {
    size_t charArrayLength = strlen(charArray);
    if (charArrayLength > BUFFER_SIZE) {
        // the charArray is too big
        return false;
    }
    
    memset(*resultingString, 0, BUFFER_SIZE);
    memcpy(*resultingString, charArray, charArrayLength);

    return true;
}


size_t string_length(const String string) {
    return strlen(string);
}

char string_characterAt(const String string, size_t index) {
    if (index >= string_length(string)) {
        return (char) -1;
    }
    return string[index];
}

bool string_inserCharacterAt(String string, size_t index, char characterToInsert) {
    if (index >= _string_capacity(string)) {
        return false;
    }
    string[index] = characterToInsert;

    return true;
}

bool string_equalsCharArray(String string, char* charArray) {
    return strcmp(string, charArray) == 0;
}

size_t string_nextSpaceTokenStartingAtIndex(const String string, size_t indexToStartAt, String bufferToStoreToken) {
    size_t lengthOfString = string_length(string);
    if (indexToStartAt >= lengthOfString) {
        return indexToStartAt;
    }

    size_t index;
    for (index = indexToStartAt; string_characterAt(string, index) != ' ' && index < lengthOfString; index++) {
      bufferToStoreToken[index - indexToStartAt] = string[index];
    }
    bufferToStoreToken[index - indexToStartAt] = '\0';

    return index;
}

void string_toLower(String string) {
    for (size_t index = 0; index < string_length(string); index++) {
        char lowercasedChar = tolower(string_characterAt(string, index));
        string_inserCharacterAt(string, index, lowercasedChar);
    }
}