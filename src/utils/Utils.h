#ifndef DB9C663F_7757_4E5F_9CDD_23E9A09D9515
#define DB9C663F_7757_4E5F_9CDD_23E9A09D9515

#include <stdint-gcc.h> // For gcc
#include <stdint.h>

#define INITIAL_FEN ("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

#define BOARD_SIZE (64)
#define BOARD_LENGTH (8)
#define MAX_LEGAL_MOVES (218)
#define MAX_NB_LEGAL_MOVES_IN_GAME (6000)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t  i8;

// The x coordinates
#define file(square) (square & 7)
// The y coordinates
#define rank(square) (square >> 3)

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


// Copied from StackOverFlow https://stackoverflow.com/a/20747375
// log_2(0) is undefined
static inline int trailingZeros_64(const u64 x) {
    return __builtin_ctzll(x);
}

static inline int numBitSet_64(u64 x) {
	return __builtin_popcountll(x);
}

u64 random_u64();

#endif /* DB9C663F_7757_4E5F_9CDD_23E9A09D9515 */
