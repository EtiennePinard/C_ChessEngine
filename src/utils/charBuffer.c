#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#include "CharBuffer.h"

bool string_compareStrings(const char *string1, const char* string2) {
    if (string1[0] != string2[0]) { return false; }
    size_t length1 = strlen(string1);
    size_t length2 = strlen(string2);
    if (length1 != length2) { return false; }
    return memcmp(string1, string2, length1) == 0;
}

size_t string_nextSpaceCharacterFromIndex(const char *string, size_t indexToStartAt) {
    size_t lengthOfString = strlen(string);
    if (indexToStartAt >= lengthOfString) { return lengthOfString; }

    size_t index;
    for (index = indexToStartAt; string[index] != ' ' && index < lengthOfString; index++);
    return index;
}

void string_toLower(char *string) {
    for (size_t index = 0; index < strlen(string); index++) {
        string[index] = tolower(string[index]);
    }
}

int string_parseNumber(const char *num) {
  size_t lengthOfNum = strlen(num);
  if (lengthOfNum == 0) { return -1; }

  int result = 0;
  
  // Starting from the end
  for (int index = (int) lengthOfNum - 1; index >= 0; index--) {
    char character = num[index];
    if (character < '0' || character > '9') { return -1; }
    
    int numToAdd = character - '0';
    for (int pow = index; pow < (int) lengthOfNum - 1; pow++) {
      numToAdd *= 10;
    }
    
    result += numToAdd;
  }
  
  return result;
}

int string_algebraicToIndex(const char *algebraic) {
  size_t lengthOfSquare = strlen(algebraic);
  if (lengthOfSquare == 0) { return -1; }
  if (algebraic[0] == '-') { return 0; }
  if (lengthOfSquare != 2) { return -1; }

  if (algebraic[0] < 'a' || algebraic[0] > 'h') { return -1; }
  if (algebraic[1] < '0' || algebraic[1] > '9') { return -1; }
  int file = algebraic[0] - 'a';
  int rank = 8 - (algebraic[1] - '0');
  return rank * 8 + file;
}