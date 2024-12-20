#ifndef CELLS_PUB_H
#define CELLS_PUB_H

/** File provides implementation of crud operations on cell and block layers
 * based on "straight" way of searching objects in a tree - by dfs algorithm
 */

#include "blocks/block_info.h"
#include "blocks/block_info_pub.h"
#include "cell_types_pub.h"
#include "utils/iterators/iterators.h"


struct cell* create_cell(enum cl_type ct, const unsigned int value_length, const void* const value);

struct cell* find_cell(struct blocks_info* const, const struct cl_desc);

struct cell* select_cell(struct blocks_info* const, const struct cl_desc);

struct cl_desc insert_cell(struct blocks_info* const, const struct cell* const inserting_cell, struct cell* const found_cell_place);

struct cl_desc update_cell(struct blocks_info* const, struct cell* const updated_cell);

struct cl_desc delete_cell(struct blocks_info* const, struct cell* const deleting_cell);


// Cell object interactions (Tree operations)

struct cell* cells_get_parent(struct blocks_info* const, struct cell* cell_obj);

struct cell* cells_get_first_child(struct blocks_info* const, struct cell* parent);

cl_desc cells_insert_full_attr(struct blocks_info* const bl_info, struct cell* new_attr, struct cell* new_key,
                               struct cell* new_value, const cl_desc prev_attr);

cl_desc cells_insert_full_node(struct blocks_info* const bl_info, struct cell* node, struct cell* name, struct cell* value,
                               const cl_desc attr_chain_start_cld, const cl_desc parent_decs);


int cells_add_child(struct blocks_info* const, const cl_desc parent_cld, const cl_desc inserted_child_cld);

// int cell_obj_remove_child(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc child_cld);

struct cell* cells_get_next_sibling(struct blocks_info* const, struct cell* node);

// find last sibling if any and then add
int cells_add_sibling(struct blocks_info* const, struct cell* const cell, const struct cl_desc sibling_cld);

// int cell_obj_remove_sibling(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc sibling_cld);

int cell_add_attribute(struct blocks_info* const, struct cell* const obj, struct cell* attr);

// int cell_obj_remove_attribute();

struct cell* cells_node_get_value_cell(struct blocks_info* const, struct cell* const node);

struct cell* cells_node_get_name_cell(struct blocks_info* const, struct cell* const node);

struct cell* cells_attr_get_key_cell(struct blocks_info* const, struct cell* const attr_node);

struct cell* cells_attr_get_value_cell(struct blocks_info* const, struct cell* const attr_node);

#endif
