#ifndef CELL_TYPES_PUB_H
#define CELL_TYPES_PUB_H

#include "blocks/block_types_pub.h"


#define UNDEF_CL_DESC -1

/* Cell identifier for all references to a cell.
Cell id is translated into offset inside block at each attempt to reference a cell. 
 */
typedef short v_cl_desc; // Now it is not helping to mask cell's location if it's moved between blocks and persist valid desc 

#ifdef VIRT_CELL_DESC
/* Cell descriptor. 
bl_d - virtual block descriptor, which contains the cell
cl_d - cell identifier, which defines cell offset inside a block after translation
*/
struct cl_desc {
    v_bl_desc bl_d;
    v_cl_desc cl_d;
};
#else
typedef struct cl_desc {
    bl_desc bl_d;
    bl_offset cl_d;
} cl_desc;
#endif

struct cell;

int cells_cmp_cl_desc(const cl_desc first, const cl_desc second);

int cells_cmp(const struct cell* const first, const struct cell* const second);


typedef union cells_simple_values {
    int int32;
    bool boolean;
    float float32;
    char* string;
} cells_simple_values;

cells_simple_values cells_get_simple_type_value(struct cell*);


#define CELL_TYPES_AMOUNT 8

enum cl_type {
    CELL_UNDEF = 0,
    CELL_INT32,
    CELL_FLOAT32,
    CELL_BOOL,
    CELL_STRING,
    CELL_BLOCK,
    CELL_META,
    CELL_OBJECT,
    CELL_ATTR,
};

struct cl_desc cells_get_default_cld(void);

size_t cells_get_data_type_size(const enum cl_type ct);

size_t cells_get_cell_header_size(enum cl_type ct);

size_t cells_get_cell_type_size(const enum cl_type ct);

bl_offset cells_get_cell_offset_by_index(const enum cl_type ct, const size_t cl_index);

enum bl_type cells_get_bl_type_by_cl_type(const enum cl_type cl_type);


size_t cells_get_cell_size(const struct cell* const cell);

enum cl_type cells_get_cell_type(const struct cell* const cell);

cl_desc cells_get_cell_desc(const struct cell* const cell);

#endif
