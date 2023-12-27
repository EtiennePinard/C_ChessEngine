#ifndef C9DC8578_DB32_4605_B1DE_4C2AD8D5BE0C
#define C9DC8578_DB32_4605_B1DE_4C2AD8D5BE0C

#include <stddef.h>

/**
 * A dynamic array is a "fat" pointer, meaning that it is a pointer 
 * that, along with the data, also has the capacity and the current index.
 * The following struct below is a "generic" template that you can use
 * to create your own dynamic array.
*/
typedef struct GenericDynamicArray {
    void* items;
    size_t count;
    size_t capacity;
} GBA;

/**
 * Macro that you use to append items to your dynamic array.
 * Copy and pasted from https://github.com/tsoding/ded/blob/master/src/common.h#L45
*/
#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

#endif /* C9DC8578_DB32_4605_B1DE_4C2AD8D5BE0C */
