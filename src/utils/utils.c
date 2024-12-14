#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
inline int trailingZeros_64(const u64 x);

inline int numBitSet_64(u64 x);

u64 random_u64() {
  u64 u1 = (u64)(rand()) & 0xFFFF; 
  u64 u2 = (u64)(rand()) & 0xFFFF;
  u64 u3 = (u64)(rand()) & 0xFFFF; 
  u64 u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

u8 nextSpaceTokenStartingAtIndex(const char str[BUFFER_SIZE], u8 indexToStartAt, char bufferToStoreToken[BUFFER_SIZE]) {
    u8 index;
    for (index = indexToStartAt; str[index] != ' ' && str[index] != '\0'; index++) {
      bufferToStoreToken[index - indexToStartAt] = str[index];
    }
    bufferToStoreToken[index - indexToStartAt] = '\0';

    return index;
}