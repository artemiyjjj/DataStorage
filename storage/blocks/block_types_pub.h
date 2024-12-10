#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

#include <glib.h>
#include <stdbool.h>

// Number to identify a block and interract with it.
typedef long int v_bl_desc;

// Number to identify a block by its position in storage.
typedef long int bl_desc;

// Offset from the start of a block contents.
typedef int bl_offset;

// Amount of free cells or free bytes (for non-fixed cell blocks)
typedef unsigned int bl_free_space;


#define UNDEF_BL_DESC (-1)

#define UNDEF_BL_OFFSET -1


enum bl_type {
    BLOCK_UNDEF = 0,
    BLOCK_HEAD,
    BLOCK_DATA_FIX,
    BLOCK_DATA_DYN,
    BLOCK_META,
    // ? BLOCK_NAMES,
    // ...
};

/**
 * @brief Mode of accessing a cell in a block. Used for
 * allowing read or update of a cell in case it's already
 * in use.
 */
enum cl_pin_mode {
    CL_PIN_READ = 1,
    CL_PIN_WRITE
};

// In-memory representation of block of any type
struct block;

size_t blocks_get_block_header_size(const enum bl_type bl_type);


#endif
