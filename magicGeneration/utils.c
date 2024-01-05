#include <stdlib.h>
#include "Utils.h"

u64 random_u64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

u64 random_u64_fewbits() {
  return random_u64() & random_u64() & random_u64();
}

// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
int trailingZeros_64(const u64 x) {
    return __builtin_ctzl(x);
}

int nbBitsSet(u64 num) {
  int result;
  while(num) {
    result++;
    num &= num - 1;
  }
  return result;
}