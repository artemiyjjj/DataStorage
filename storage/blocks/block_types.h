#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

#include <stdbool.h>

// Number to identify a block by its position in storage.
typedef long int bl_desc;

// Offset from the origin of a block.
typedef long int bl_offset;

#define UNDEF_BL_DESC -1
#define UNDEF_BL_OFFSET -1

enum bl_type {
    BLOCK_HEAD = 0,
    BLOCK_DATA_FIX,
    BLOCK_DATA_DYN,
    // ? BLOCK_NAMES,
    // ...
};

#endif
