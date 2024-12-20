#include "blocks/block_info_pub.h"
#include "blocks/block_types.h"
#include "blocks/block_types_pub.h"
#include "cells/cell_types_pub.h"
#include "cells/cells.h"
#include "cells_pub.h"
#include "cell_types.h"
#include "utils/iterators/iterators.h"

#include <assert.h>
#include <time.h>

struct cell* cells_get_parent(struct blocks_info* const bl_info, struct cell* cell_obj) {
    return select_cell(bl_info, cell_obj -> ptr.cl_object -> parent);
}

struct cell* cells_get_first_child(struct blocks_info* const bl_info, struct cell* parent) {
    return select_cell(bl_info, parent -> ptr.cl_object -> children);
}


// static struct cell* get_last_sibling(struct blocks_info* const bl_info, struct cell* node) {

// }

/**
 * @brief Insert cells key and value, then insert cell attr with valid inserted descriptors of key and value 
 * 
 * @param bl_info 
 * @param new_attr 
 * @param new_key 
 * @param new_value 
 * @return cl_desc 
 */
cl_desc cells_insert_full_attr(struct blocks_info* const bl_info, struct cell* new_attr, struct cell* new_key, struct cell* new_value, const cl_desc prev_attr_desc) {
    struct cell* found_key_cell, *found_value_cell, *found_attr_cell = NULL;
    cl_desc found_key_cld, found_value_cld, found_attr_cld;
    int find_key_res, find_value_res, find_attr_res;
    struct cell* prev_cell = NULL;
    
    assert(new_key->type == CELL_STRING && new_attr->type == CELL_ATTR);

    find_key_res = storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, new_key, &found_key_cell);
    find_attr_res = storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, new_attr, &found_attr_cell);
    if (find_key_res != 0 || find_attr_res != 0) {
        return cells_get_default_cld();
    }
    find_value_res = new_value ? storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, new_value, &found_value_cell) : 0;
    found_key_cld = insert_cell(bl_info, new_key, found_key_cell);
    found_value_cld = find_value_res == 0 ? insert_cell(bl_info, new_value, found_value_cell) : cells_get_default_cld();
    // Update attr cell's descriptors of key and value
    new_attr -> ptr.cl_attribute -> key = found_key_cld;
    new_attr -> ptr.cl_attribute -> value = found_value_cld;
    
    found_attr_cld = insert_cell(bl_info, new_attr, found_attr_cell);
    if (cells_cmp_cl_desc(prev_attr_desc, cells_get_default_cld()) != 0) {
        cl_desc updated_prev_cld;
        prev_cell = find_cell(bl_info, prev_attr_desc);
        prev_cell -> ptr.cl_attribute->next_attr = found_attr_cld;
        updated_prev_cld = update_cell(bl_info, prev_cell);
        if (cells_cmp_cl_desc(prev_attr_desc, updated_prev_cld) != 0) {
            return cells_get_default_cld();
        }
    }
    return found_attr_cld;
}

/**
 * @brief Insert cells name and value, set inserted descriptors of these and attrs
 * 
 * @param bl_info 
 * @param node 
 * @param name 
 * @param value 
 * @param attr_cells_arr_iter 
 * @return cl_desc 
 */
cl_desc cells_insert_full_node(struct blocks_info* const bl_info, struct cell* node, struct cell* name, struct cell* value,
                                 const cl_desc attr_chain_start_cld, const cl_desc parent_cld)
{
    struct cell* found_cell_name, *found_cell_value, *found_cell_node;
    int find_name_res, find_value_res, find_node_res;
    cl_desc inserted_name_cld, inserted_node_cld, inserted_value_cld = cells_get_default_cld();

    if (cells_cmp_cl_desc(parent_cld, cells_get_default_cld()) == 0) {
        return cells_get_default_cld();
    }

    assert (node -> type == CELL_OBJECT && name -> type == CELL_STRING);
    find_name_res = storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, name, &found_cell_name);
    find_value_res = value != NULL ? storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, value, &found_cell_value) : 0;
    find_node_res = storage_find_new_cell_place(bl_info, UNDEF_BL_DESC, node, &found_cell_node);
    if ((find_name_res | find_value_res | find_node_res) != 0) {
        // free other cells
        return cells_get_default_cld();
    }
    inserted_name_cld = insert_cell(bl_info, name, found_cell_name);
    inserted_value_cld = value != NULL ? insert_cell(bl_info, value, found_cell_value) : cells_get_default_cld();
    node -> ptr.cl_object -> attrs = attr_chain_start_cld;
    node -> ptr.cl_object -> name = inserted_name_cld;
    node -> ptr.cl_object -> value = inserted_value_cld;
    node -> ptr.cl_object -> parent = parent_cld;
    inserted_node_cld = insert_cell(bl_info, node, found_cell_node);

    if (cells_add_child(bl_info, parent_cld, inserted_node_cld) != 0) {
        return cells_get_default_cld();
    }
    return inserted_node_cld;
}

/**
 * @brief Insert an already inserted child cell 
 * 
 * @param bl_info 
 * @param parent 
 * @param child 
 * @return int 0 - Success
 * @return int 1 - Failed to find a place for a new cell
 * @return int 2 - Failed to insert a new cell to the found one
 * @return int 3 - Failed to select one of child cells
 */
int cells_add_child(struct blocks_info* const bl_info, const cl_desc parent_cld, const cl_desc inserted_child_cld) {
    struct cell* last_child_cell = NULL;
    struct cell* parent_cpy = select_cell(bl_info, parent_cld);
    struct cell* inserted_child = find_cell(bl_info, inserted_child_cld);
    bool is_insert_to_parent;

    if (inserted_child -> type != CELL_OBJECT) {
        return -1;
    }
    inserted_child -> ptr.cl_object -> parent = parent_cld;

    // Find a node where to put new child 
    // Check if any children are present
    if (cells_cmp_cl_desc(parent_cpy -> ptr.cl_object -> children, cells_get_default_cld()) == 0) {
        is_insert_to_parent = true; // add free_cell(parent_cpy)
        last_child_cell = find_cell(bl_info, parent_cld);
    } else {
        is_insert_to_parent = false;
        // Get children list
        last_child_cell = find_cell(bl_info, parent_cpy -> ptr.cl_object -> children);
        if (last_child_cell == NULL) {
            return 3;
        }
        // Find last sibling
        while(last_child_cell -> ptr.cl_object -> next_sibling.bl_d != UNDEF_BL_DESC && last_child_cell -> ptr.cl_object -> next_sibling.cl_d != UNDEF_CL_DESC) {
            struct cell* tmp_cell = cells_get_next_sibling(bl_info, last_child_cell);
            free(last_child_cell);
            last_child_cell = tmp_cell;
        }
        assert(last_child_cell);
    }
    
    
    if (cells_cmp_cl_desc(inserted_child_cld, cells_get_default_cld()) == 0) {
        return 2;
    }
    if (is_insert_to_parent) {
        last_child_cell -> ptr.cl_object -> children = inserted_child_cld;
    } else {
        last_child_cell -> ptr.cl_object -> next_sibling = inserted_child_cld;
    }
    update_cell(bl_info, last_child_cell);
    update_cell(bl_info, inserted_child);
    return 0;
}

// int cell_obj_remove_child(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc child_cld);

struct cell* cells_get_next_sibling(struct blocks_info* const bl_info, struct cell* node) {
    return find_cell(bl_info, node -> ptr.cl_object -> next_sibling);
}

// find last sibling if any and then add
int cells_add_sibling(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc sibling_cld) {
    return -1;
}

// int cell_obj_remove_sibling(struct blocks_info* const bl_info, struct cell* const cell, const struct cl_desc sibling_cld);

int cell_add_attribute(struct blocks_info* const bl_info, struct cell* const obj_cpy, struct cell* new_attr) {
    struct cell* last_attr_cell = NULL;
    struct cell* found_cell = NULL;
    cl_desc found_cld;
    cl_desc inserted_cld;
    bool is_insert_node;

    if (new_attr -> type != CELL_ATTR) {
        return -1;
    }
    if (obj_cpy -> ptr.cl_object -> attrs.bl_d == UNDEF_BL_DESC && obj_cpy -> ptr.cl_object -> attrs.cl_d == UNDEF_CL_DESC) {
        is_insert_node = true;
        last_attr_cell = find_cell(bl_info, obj_cpy -> cl_desc);
    } else {
        is_insert_node = false;
        last_attr_cell = find_cell(bl_info, obj_cpy -> ptr.cl_object -> attrs);
        if (!last_attr_cell) {
            return 1;
        }
        while (last_attr_cell -> ptr.cl_attribute -> next_attr.bl_d == UNDEF_BL_DESC && last_attr_cell -> ptr.cl_attribute -> next_attr.cl_d == UNDEF_CL_DESC) {
            struct cell* tmp_cell = find_cell(bl_info, last_attr_cell -> ptr.cl_attribute -> next_attr);
            free(last_attr_cell);
            last_attr_cell = tmp_cell;
        }
    }
    if (!last_attr_cell) {
        return 1;
    }

    if (0 != storage_find_new_cell_place(bl_info, last_attr_cell -> cl_desc.bl_d, new_attr, &found_cell)) {
        return 1;
    }
    found_cld = found_cell -> cl_desc;
    inserted_cld = insert_cell(bl_info, new_attr, found_cell);
    if (found_cld.bl_d != inserted_cld.bl_d || found_cld.cl_d != inserted_cld.cl_d) {
        return 2;
    }

    if (is_insert_node) {
        last_attr_cell -> ptr.cl_object -> attrs = inserted_cld;
    } else {
        last_attr_cell -> ptr.cl_attribute -> next_attr = inserted_cld;
    }
    update_cell(bl_info, last_attr_cell);
    return 0;
} 

// int cell_obj_remove_attribute();

struct cell* cells_node_get_value_cell(struct blocks_info* const bl_info, struct cell* const node) {
    return select_cell(bl_info, node -> ptr.cl_object -> value);
}

struct cell* cells_node_get_name_cell(struct blocks_info* const bl_info, struct cell* const node) {
    return select_cell(bl_info, node -> ptr.cl_object -> name);
}

struct cell* cells_attr_get_key_cell(struct blocks_info* const bl_info, struct cell* const attr_node) {
    return select_cell(bl_info, attr_node -> ptr.cl_attribute -> key);
}

struct cell* cells_attr_get_value_cell(struct blocks_info* const bl_info, struct cell* const attr_node) {
    return select_cell(bl_info, attr_node -> ptr.cl_attribute -> value);
}
