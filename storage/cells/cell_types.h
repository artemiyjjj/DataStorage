#ifndef CELL_TYPES_H
#define CELL_TYPES_H

#include "blocks/block_types_pub.h"
#include "cell_types_pub.h"

#include <stddef.h>
#include <stdbool.h>


#define UNDEF_CL_SIZE 0

/**
 * @brief Common wrapper for cell contents.
 * Each cell can have its own contents, which structure is  predefined in program.
 * Length of each cell is defined nor by the static data block in its header,
 * nor by the `length` field in cell structure, if its length is variable
 */
typedef struct cell_common {
    enum cl_type type;
    char cell_content[];
}__attribute__((packed)) cell_common;

// Simple types

typedef struct cell_int32 {
    enum cl_type type;
    int value;
}__attribute__((packed)) cell_int32;

typedef struct cell_float32 {
    enum cl_type type;
    float value;
}__attribute__((packed)) cell_float32;

typedef struct cell_bool {
    enum cl_type type;
    bool value;
}__attribute__((packed)) cell_bool;

typedef struct cell_string {
    enum cl_type type;
    unsigned int length;
    char value[];
}__attribute__((packed)) cell_string;

// End of simple types

// Object types

/**
 * @brief Cell for an object's attribute name.
 * Contains descriptor of a `cl_string` cell which is the attribute's name
 * and descriptor of a value cell which can be a cell of any simple type or
 * an undefined cell descriptor when attr has just key.
 */
typedef struct cell_attr {
    enum cl_type type;
    cl_desc key;
    cl_desc value;
    cl_desc next_attr;
}__attribute__((packed)) cell_attr;

/**
 * @brief Cell for an object representation in storage.
 * 
 * Object optionaly can hold a value of simple type.
 * If object is of simple type itself then type attribute is overhead,
 * else type is essential and object couldn't be created without it.
 * 
 * Fields `attrs` and `children` are of type struct `cl_contents_collection`,
 * `value` is of any Simple type.
 */
typedef struct cell_object {
    enum cl_type type;
    struct cl_desc attrs;
    struct cl_desc children;
    struct cl_desc next_sibling;
    struct cl_desc parent;
    struct cl_desc value;
    struct cl_desc name;
}__attribute__((packed)) cell_object;

/**
 * @brief Collection of descriptors for attributes or anchestors of an object 
 * 
 * Takes more time to fetch attributes/children but gives access to all elements at once[is it usefull??] (might add value type of attr with cl_desc)
 */
// struct cell_contents_collection {
//     enum cl_type type;
//     // int contents_amount;
//     struct cl_desc contents_desc[]; // change to linked list behaviour
// };


// End of Object types

// Internal types

/**
 * @brief Relation between real and virtual descriptor of a block in storage,
 * amount of free space (or free cells) and blocks's storing cell type to be
 * able to create stuctures for finding a place where to put new cells, without
 * reading each block to get with information.
 */
typedef struct cell_block {
    v_bl_desc     vbd;
    bl_desc       rbd;
    bl_free_space free_bl_size;
    enum cl_type  stor_cl_type;
}__attribute__((packed)) cell_block;

/**
 * @brief Relation between virtual cell descriptor and offset of a cell inside
 * a block.
 *
 * This cell does not need `type` field, since it's used only in `block_meta`
 */
typedef struct cell_meta {
    v_cl_desc virt_cl_desc;
    bl_offset bl_off;
    // cell's data hash
}__attribute__((packed)) cell_meta;

// End of Internal types

typedef struct cell {
    enum cl_type type;
    struct cl_desc cl_desc;
    union {
        void*                common;
        struct cell_common*  cl_common;
        struct cell_int32*   cl_int32;
        struct cell_float32* cl_float32;
        struct cell_bool*    cl_bool;
        struct cell_string*  cl_string;
        struct cell_object*  cl_object;
        struct cell_attr*    cl_attribute;
        struct cell_block*   cl_block;
        struct cell_meta*    cl_meta;
        // ...
    } ptr;
} cell;

#endif
