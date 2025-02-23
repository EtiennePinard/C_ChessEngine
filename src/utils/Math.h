#ifndef DB9C663F_7757_4E5F_9CDD_23E9A09D9515
#define DB9C663F_7757_4E5F_9CDD_23E9A09D9515

#include "Types.h"

// The x coordinates
#define file(square) (square & 7)
// The y coordinates
#define rank(square) (square >> 3)

/**
 * @brief Returns the maximum of a and b.
 * If a < b it returns b.
 * If a > b it returns a.
 * If a == b it returns b.
 */
#define max(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/**
 * @brief Returns the minimum of a and b.
 * If a < b it returns b.
 * If a > b it returns a.
 * If a == b it returns b.
 */
#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/**
 * @brief Computes the number of trailing zeroes that a number contains in binary
 * 
 * Examples: 
 *          trailingZeros_64(96)   = trailingZeros_64(0b1100000)        = 5
 *          trailingZeros_64(431)  = trailingZeros_64(0b110101111)      = 0
 *          trailingZeros_64(8192) = trailingZeros_64(0b10000000000000) = 13
 *  
 * Note: trailingZeros_64(0) is undefined since it uses the ctzll function, which does 
 * define output for zero values. 
 * 
 * @param x The number to get the trailing zeroes of
 * @return int The number of trailing zeroes of x
 */
static inline int trailingZeros_64(const u64 x) {
  return __builtin_ctzll(x);
}

/**
 * @brief Computes the number of 1s in the binary representation of a number.
 * 
 * Examples: 
 *          numBitSet_64(96)   = numBitSet_64(0b1100000)        = 2
 *          numBitSet_64(431)  = numBitSet_64(0b110101111)      = 7
 *          numBitSet_64(8192) = numBitSet_64(0b10000000000000) = 1
 * 
 * @param x 
 * @return int 
 */
static inline int numBitSet_64(u64 x) {
  return __builtin_popcountll(x);
}

#endif /* DB9C663F_7757_4E5F_9CDD_23E9A09D9515 */
