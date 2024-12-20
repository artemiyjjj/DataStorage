#include "cells_iterators.h"

#include "blocks/block_info.h"
#include "blocks/block_info_pub.h"
#include "blocks/block_types_pub.h"
#include "cells/cells.h"
#include "cells_pub.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"
#include "glib.h"
#include "utils/iterators/iterators.h"
#include "utils/mem.h"

#include <assert.h>
#include <stdbool.h>


iterator* new_root_cell_iterator(struct blocks_info* const bl_info, struct cell* const root_cell) {
    root_cell_iterator* root_cell_iter = myAllocStruct(root_cell_iterator);
    if (root_cell_iter == NULL) {
        return NULL;
    }
    root_cell_iter -> stor_bl_info = bl_info;
    root_cell_iter -> root_cell = root_cell;
    root_cell_iter -> base.current = NULL;
    root_cell_iter -> base.move_next = root_cell_move_next;
    root_cell_iter -> base.dstr_item = NULL;
    return (iterator*) root_cell_iter;
}

/**
 * @brief Create data block and set iterator to 
 *
 * Cell wrap in iterator.current should be manually freed using `release_cell_ptr`
 * 
 * @param self 
 * @return true 
 * @return false 
 */
bool root_cell_move_next(iterator* self) { // this can be generic for all `move_next`
    int                 find_cell_place_res;
    root_cell_iterator* cast_self = (root_cell_iterator*) self;
    struct cell*        found_cl = NULL;

    // Invalidate iterator then finish iteration
    if (found_cl != NULL) {
        free(self);
        return false;
    }

    find_cell_place_res = storage_find_new_cell_place(cast_self -> stor_bl_info, UNDEF_BL_DESC, cast_self -> root_cell, &found_cl);
    if (find_cell_place_res != 0) {
        return false;
    }
    self -> current = found_cl;
    return true;
}

/// These iterators should return struct cell 

static bool test_attr_name(void* name_attr, void* expected_name) {
    return strcmp(name_attr, expected_name) == 0 ? true : false;
}

/**
 * @brief Create an iterator over a single cell. Cell should be accessed via
 * `select_cell` to get a copy of desired cell.
 * 
 * @param node 
 * @return iterator* 
 */
iterator* new_single_item_iterator(struct cell* const item) {
    void** items = myAllocStruct(void*);
    items[0] = item;
    return new_array_iterator(items, 1, free);
}

static void cell_cpy_dstr(void* cell) {
    cells_free_cpy_cell(cell);
}

/**
 * @brief Create an iterator over a cell's only immediate (next level)
 * children.
 *
 * @param bl_info 
 * @param node 
 * @return iterator* 
 */
iterator* new_node_imm_children_iterator(struct blocks_info* const bl_info, struct cell* node) {
    children_iterator* self = myAllocStruct(children_iterator);
    if (!self) {
        return NULL;
    }

    self -> base.current = NULL;
    self -> base.move_next = imm_children_iter_move_next;
    self -> base.dstr_item = cell_cpy_dstr; 
    self -> next_cell_desc = node -> ptr.cl_object -> children;
    self -> bl_info = bl_info;
    return (iterator*) self;
}

bool imm_children_iter_move_next(iterator* it) {
    children_iterator* self = (children_iterator*) it;
    struct cl_desc next_cld = self -> next_cell_desc;
    struct cell* cur_cell = self -> base.current;
    
    if (cur_cell != NULL) {
        self -> base.dstr_item(cur_cell);
    }
    if (cells_cmp_cl_desc(next_cld, cells_get_default_cld()) == 0) {
        free(it);
        return false;
    }
    cur_cell = select_cell(self -> bl_info, next_cld);
    assert(cur_cell); // descriptor can not be invalid
    self -> next_cell_desc = cur_cell -> ptr.cl_object -> next_sibling;
    self -> base.current = cur_cell;
    return true;
}

/**
 * @brief Create an iterator over all cell's ancestors.
 *
 * @param node 
 * @return iterator* 
 */
iterator* new_node_all_children_iterator(struct blocks_info* const bl_info, struct cell* node) {
    all_children_iterator* self = myAllocStruct(all_children_iterator);
    if (!self) {
        return NULL;
    }

    iterator* root_children_iter = new_node_imm_children_iterator(bl_info, node);

    self -> base.current = NULL;
    self -> base.move_next = all_children_iter_move_next;
    self -> base.dstr_item = cell_cpy_dstr;
    self -> bl_info = bl_info;
    self -> iterator_stack = g_queue_new();
    g_queue_push_head(self -> iterator_stack, root_children_iter);
    return (iterator*) self;
}

bool all_children_iter_move_next(iterator* it) {
    all_children_iterator* self = (all_children_iterator*) it;

    while (g_queue_get_length(self -> iterator_stack) > 0) {
        iterator* current_iterator = g_queue_peek_tail(self -> iterator_stack);
        while (current_iterator -> move_next(current_iterator)) {
            self -> base.current = current_iterator -> current;
            iterator* cur_node_imm_children_iter = new_node_imm_children_iterator(self -> bl_info, self -> base.current);
            g_queue_push_head(self -> iterator_stack, cur_node_imm_children_iter);
            return true;
        }
        self -> iterator_stack = g_queue_pop_tail(self -> iterator_stack);
    }
    g_queue_free(self -> iterator_stack);
    free(it);
    return false;
}


iterator* new_node_attr_iterator(struct blocks_info* const bl_info, struct cell* node) {
    children_iterator* self = myAllocStruct(children_iterator);
    if (!self) {
        return NULL;
    }

    self -> base.current = NULL;
    self -> base.move_next = attr_iter_move_next;
    self -> base.dstr_item = cell_cpy_dstr;
    self -> next_cell_desc = node -> ptr.cl_object -> attrs;
    self -> bl_info = bl_info;   
    return (iterator*) self;
}

bool attr_iter_move_next(iterator* it) {
    children_iterator* self = (children_iterator*) it;
    struct cl_desc next_cld = self -> next_cell_desc;
    struct cell* cur_cell = self -> base.current;

    if (cur_cell != NULL) {
        self -> base.dstr_item(cur_cell);
    }
    if (next_cld.bl_d == UNDEF_BL_DESC && next_cld.cl_d == UNDEF_CL_DESC) {
        free(it);
        return false;
    }
    cur_cell = select_cell(self -> bl_info, next_cld);
    self -> next_cell_desc = cur_cell -> ptr.cl_attribute -> next_attr;
    self -> base.current = cur_cell;
    return true;
}


iterator* new_attr_name_filter_iterator(iterator* base, struct blocks_info* const bl_info, char* expected_name) {
    attr_filter_name_iterator* iter = myAllocStruct(attr_filter_name_iterator);
    if (iter == NULL) {
        return NULL;
    }
    iter -> bl_info = bl_info;
    iter -> base.from = base;
    iter -> base.param = expected_name;
    iter -> base.test_condition = test_attr_name;
    iter -> base.self.current = NULL;
    iter -> base.self.move_next = attr_name_iter_move_next;
    iter -> base.self.dstr_item = cell_cpy_dstr;
    return (iterator*) iter;
}

bool attr_name_iter_move_next(iterator* it) {
    struct cell* cur_cell;
    struct cell* name_cell;
    attr_filter_name_iterator* name_iter = (attr_filter_name_iterator*) it;
    children_iterator* from_iter = (children_iterator*) name_iter -> base.from;

    if (from_iter -> base.current != NULL) {
        name_iter -> base.self.dstr_item(from_iter -> base.current);
    }
    while (from_iter -> base.move_next((iterator*) from_iter)) {
        cur_cell = from_iter -> base.current;
        name_cell = select_cell(name_iter -> bl_info, cur_cell -> ptr.cl_attribute -> key);
        assert(name_cell != NULL);
        // if (name_cell == NULL) {
        //     cells_free_cpy_cell(name_cell);
        //     continue;
        // }
        /// Check if name matches expected name
        if (name_iter -> base.test_condition(name_cell -> ptr.cl_string -> value, name_iter -> base.param)) {
            cells_free_cpy_cell(name_cell);
            name_iter -> base.self.current = cur_cell;
            return true;
        }
    }
    free(from_iter);
    free(name_iter);
    return false;
}
