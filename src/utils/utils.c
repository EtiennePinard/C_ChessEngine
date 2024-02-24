#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
inline int trailingZeros_64(const u64 x) {
    return __builtin_ctzll(x);
}

inline int numBitSet_64(u64 x) {
	return __builtin_popcountll(x);
}

u64 random_u64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

void initializeFFILogging(void (*printCallback)(char *)) {
    loggingFFI = printCallback;
    loggingFFI("C library initialized");
}