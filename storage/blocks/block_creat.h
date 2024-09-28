#ifndef BLOCK_CREAT_H
#define BLOCK_CREAT_H

#include "block_info.h"
#include "block_types.h"
#include "cells/cell_types.h"

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
    struct cl_desc root_cl_desc;
    // bl_desc root_cell_desc;
    // bl_offset root_cell_offset;
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
    enum bl_type bt;

};

struct block_data_dyn {
    enum bl_type bt;
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

int create_block_struct(struct blocks_info* const bl_info, enum bl_type bt, void** const block_addr);


#endif
