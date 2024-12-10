#include "cells_iterators.h"
#include "blocks/block_info.h"
#include "blocks/block_info_pub.h"
#include "blocks/block_types_pub.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"
#include "utils/iterators/iterators.h"
#include "utils/mem.h"

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
    root_cell_iter -> base.dstr_item = iterator_empty_destroy;
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



iterator* new_array_iterator(void* arr_start, const size_t arr_len, const size_t arr_elem_size, fp_destroy_item arr_dstr) {
    array_iterator* arr_it = myAllocStruct(array_iterator);
    if (!arr_it) {
        return NULL;
    }
    arr_it -> arr_start = arr_start;
    arr_it -> arr_lenght = arr_len;
    arr_it -> arr_elem_size = arr_elem_size;
    arr_it -> next_index = 0;
    arr_it -> base.current = NULL;
    arr_it -> base.move_next = array_iter_move_next;
    arr_it -> base.dstr_item = arr_dstr;
    return (iterator*) arr_it;
}

iterator* new_cell_type_array_iter(void) {
    size_t cell_types_amount = CELL_TYPES_AMOUNT;
    array_iterator* new_cell_type_iter = myAllocStruct(array_iterator);
    if (new_cell_type_iter == NULL) {
        return NULL;
    }
    enum cl_type* arr = malloc(sizeof(enum cl_type) * cell_types_amount); 
    if (arr == NULL) {
        return NULL;
    }
    arr[0] = CELL_INT32;
    arr[1] = CELL_FLOAT32;
    arr[2] = CELL_BOOL;
    arr[3] = CELL_STRING;
    arr[4] = CELL_BLOCK;
    arr[5] = CELL_META;
    arr[6] = CELL_OBJECT;
    arr[7] = CELL_ATTR;
    new_cell_type_iter -> next_index = 0;
    new_cell_type_iter -> arr_start = arr;
    new_cell_type_iter -> arr_lenght = cell_types_amount;
    new_cell_type_iter -> arr_elem_size = sizeof(enum cl_type);
    new_cell_type_iter -> base.current = NULL;
    new_cell_type_iter -> base.move_next = array_iter_move_next;
    new_cell_type_iter -> base.dstr_item = iterator_empty_destroy;
    return (iterator*) new_cell_type_iter;
}

bool array_iter_move_next(iterator *it) {
    array_iterator* self = (array_iterator*) it;
    if (self -> next_index < self -> arr_lenght) {
        self -> base.current = (char*) self -> arr_start + self -> arr_elem_size * self -> next_index;
        self -> next_index++;
        return true;
    } else {
        self -> base.dstr_item(self -> arr_start);
        free(it);
        return false;
    }
}



/**
 * @brief Depth-first search of cells fitting 
 * provided conditions
 * 
 * @return int 0 - Success
 */
// int find_cells_dfs(struct blocks_info* bl_info, ..., const struct cell_obj* const search_start ) { // mb move to storage - select()?
//     struct cellIterator cellIterator = NULL;
//     search_start -> children


//     return -1;
// }


/// Usage
// if (find_cell(..., cellIterator) != 0) {
//         return 3;
//     } 
//     while (cellIterator.hasNext()) {
//         cellIterator
//     }

// int find_cells_meta(...){}
