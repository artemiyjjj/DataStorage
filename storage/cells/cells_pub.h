#ifndef CELLS_PUB_H
#define CELLS_PUB_H

/** File provides implementation of crud operations on cell and block layers
 * based on "straight" way of searching objects in a tree - by dfs algorithm
 */

#include "blocks/block_info_pub.h"
#include "cell_types_pub.h"


struct cell* create_cell(enum cl_type ct, const unsigned int value_length, const void* const value);

struct cell* find_cell(struct blocks_info* const, const struct cl_desc);

struct cell* select_cell(struct blocks_info* const, const struct cl_desc);

struct cl_desc insert_cell(struct blocks_info* const, const struct cell* const inserting_cell, struct cell* const found_cell_place);

struct cl_desc update_cell(struct blocks_info* const, struct cell* const updated_cell);

struct cl_desc delete_cell(struct blocks_info* const, struct cell* const deleting_cell);


// Cell object interactions (Tree operations)

// int cell_obj_add_child(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc child_cld);

// int cell_obj_remove_child(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc child_cld);

// int cell_obj_add_sibling(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc sibling_cld);

// int cell_obj_remove_sibling(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc sibling_cld);

// int cell_obj_add_attribute();

// int cell_obj_remove_attribute();

// int cell_obj_update_

#endif
