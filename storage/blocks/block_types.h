#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

#include <stdbool.h>

// Number to identify a block by its position in storage.
typedef long int bl_desc;

// Offset from the origin of a block.
typedef long int bl_offset;

#define UNDEF_BL_DESC -1
#define UNDEF_BL_OFFSET -1
#define UNDEF_CL_SIZE 0

enum bl_type {
    BLOCK_HEAD = 0,
    BLOCK_DATA_FIX,
    BLOCK_DATA_DYN,
    // ? BLOCK_NAMES,
    // ...
};

// block might not store its bl_desc, cuz its known when its loaded


/* Block for storing information about blocks in storage.
This information needs to be stored for fast inserting
after storage initialization.
For each block in storage it saves block type and amount
of free cells or bytes (depends on block type).
Block descriptors are located in ascending order.
Next header block is created when first header block runs
out of space for information about new blocks.
*/
struct block_header {
    enum bl_type type;
    // Root cell desc
    bl_desc root_cell_desc;
    bl_offset root_cell_offset;
    bl_desc next_header_block_desc;
    struct {
        long int last_bl_desc;
        unsigned int free_cells;
    } state;
    unsigned char cell_size;
    char block_content[];
};

/* Block for storing data of types, which has fixed length. 
For example INT and BOOL have fixed length and STRING has variable length.
*/
struct block_data_fix {
    // ...
};

struct block_data_dyn {
    // ...
};

//...

struct block {
    enum bl_type type;
    bl_desc desc;
    bool dirty;
    union {
        struct block_header* bl_header;
        //...
    } ptr;
};

#endif
