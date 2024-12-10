#ifndef BLOCK_CREAT_H
#define BLOCK_CREAT_H

#include "block_types_pub.h"
#include "cells/cell_types_pub.h"

#define BLOCK_HEADER_MAGIC 0x6A6A


typedef struct block_state {
    bl_free_space free_cells;
    bl_offset new_cell_start;
}__attribute__((packed)) block_state;

/* Block for storing information about blocks in storage.
This information needs to be stored for fast inserting
after storage initialization.
For each block in storage it saves block type and amount
of free cells or bytes (depends on block type).
Block descriptors are located in ascending order.
Next header block is created when first header block runs
out of space for information about new blocks.

? No block_meta descriptor required because cells are
located sequentially (but may be deleted with block before
freed space is captured by other block)
*/
typedef struct block_header {
    enum bl_type type;
    char magic[2];
    struct cl_desc root_cl_desc;
    struct block_state state;
    bl_desc last_bl_desc;
    enum cl_type data_type;
    char block_content[];
}__attribute__((packed)) block_header;

/**
 * @brief Block structure for storing meta-information of some 
 * other block (or multiple blocks) 
 * 
 */
typedef struct block_meta {
    enum bl_type type;
    enum cl_type data_type;
    // v_bl_desc parent_bl_desc; // not good if keeps info about multiple blocks
    struct block_state state;
    char block_content[];
}__attribute__((packed)) block_meta;

/* Block for storing data of types, which has fixed length. 
For example INT and BOOL have fixed length and STRING has variable length.
*/
typedef struct block_data_fix {
    enum bl_type type;
    enum cl_type data_type;
    // bl_desc block_meta_desc; // - corresponding block_meta block
    struct block_state state;
    char block_content[];
}__attribute__((packed)) block_data_fix;

typedef struct block_data_dyn {
    enum bl_type type;
    enum cl_type data_type;
    // bl_desc block_meta_desc; // - corresponding block_meta block
    struct block_state state;
    char block_contents[];
}__attribute__((packed)) block_data_dyn;

//...

typedef struct pin_counter {
    int cl_read_counter;
    int cl_write_counter;
} pin_counter;


/**
 * @brief Representation of any block in storage that has been loaded into memory
 * 
 * @field is_dirty - indicates if block contents were modified since it's
 * loaded in memory and does it needs to be synced with the storage
 * 
 */
typedef struct block {
    enum bl_type type;
    v_bl_desc v_desc;
    bl_desc real_desc;
    bool is_dirty;
    // <bl_offset, struct pin_counter<int, int>>
    GHashTable* cell_pins2pin_counter;
    /// also would allow to add mutexes on pinned cells if they pinned for update or insert
    union {
        void* common;
        struct block_header* bl_header;
        struct block_meta* bl_meta;
        struct block_data_fix* bl_data_fix;
        struct block_data_dyn* bl_data_dyn;
        //...
    } ptr;
} block;

#endif
