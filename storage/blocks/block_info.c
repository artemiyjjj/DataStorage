#include "block_info.h"

#include "block_info_pub.h"
#include "blocks_pub.h"
#include "block_types.h"
#include "block_types_pub.h"
#include "cells/cells.h"
#include "cells/cells_pub.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>


// Operations with meta-info

// Todo: Change both funcs to support cell_blocks and cell_meta 

/**
 * @brief Get copy of cell_block with info about requested block
 * 
 * @param bl_info 
 * @param rbd 
 * @param cl_meta 
 * @return int 0 - Success
 * @return int 1 - Failed to find header for provided block (Out of bounds)
 * @return int 2 - Failed to get cell
 */
int storage_get_meta_info_of_block(struct blocks_info* const bl_info, const bl_desc rbd, struct cell** const cl_meta) {
    bl_desc         header_bl_desc;
    size_t          bl_position_in_header_group;
    struct cl_desc  meta_cl_desc;

    header_bl_desc = storage_get_header_desc_of(bl_info -> block_size, rbd);
    bl_position_in_header_group = storage_get_bl_position_in_header_group(bl_info -> block_size, rbd);
    // Descriptor of cell_block in corresponding block_header
    meta_cl_desc.bl_d = header_bl_desc;
    meta_cl_desc.cl_d = cells_get_cell_offset_by_index(CELL_BLOCK, bl_position_in_header_group);
    // Get cell_block wrap from calculated block_header
    *cl_meta = select_cell(bl_info, meta_cl_desc);
    if (*cl_meta == NULL) {
        return 2;
    }
    return 0;
}

/**
 * @brief Update value in cell_block of some block in corresponding block_header
 *
 * Frees provided cell_block after update.
 * 
 * @param rbd 
 * @param cl_meta 
 * @return int 0 - Successfully modified cell with meta info
 * @return int 1 - Failed to get meta cell for modifying
 * @return int 2 - Failed to set changes to provided cell
 */
int storage_set_meta_info_of_block(struct blocks_info* const bl_info, const struct block* const modifying_bl, struct cell* cl_meta_info) {
    struct cell* modifying_block_cell = NULL;
    bool is_inserting = false;
    struct cl_desc set_cell_desc;
    
    modifying_block_cell = find_cell(bl_info, cl_meta_info -> cl_desc);
    if (modifying_block_cell == NULL) {
        // mb get status if failed due to pin
        return 1;
    }
    // Set fields if cell is not occupied
    if (cl_meta_info -> ptr.cl_block -> stor_cl_type == CELL_UNDEF) {
        is_inserting = true;
        cl_meta_info -> ptr.cl_block -> stor_cl_type = blocks_get_block_data_type(modifying_bl);
        cl_meta_info -> ptr.cl_block -> vbd = modifying_bl -> v_desc;
        cl_meta_info -> ptr.cl_block -> rbd = modifying_bl -> real_desc;
    }
    // no need to update block header's state, since it's already updated
    if (modifying_bl -> type == BLOCK_HEAD) {
        free(modifying_block_cell);
        cells_free_cpy_cell(&cl_meta_info);
        return 0;
    }
    // Update or insert `block_header`'s state and make this block dirty
    if (is_inserting) { // insert will change block's state info in block's header 
        set_cell_desc = insert_cell(bl_info, cl_meta_info, modifying_block_cell);
    } else {            // update will only set dirty
        memcpy(modifying_block_cell, cl_meta_info, cells_get_cell_size(cl_meta_info));
        set_cell_desc = update_cell(bl_info, modifying_block_cell);
    }

    if (set_cell_desc.cl_d == UNDEF_CL_DESC) {
        return 2;
    }
    // ?? how to define if cell is new or exists by cell
    // if (storage_update_block_meta_info_by_cell_operation(bl_info, modifying_block_cell, CL_UPDATE) != 0) {
    //     return 2;
    // }
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, modifying_block_cell) != 0) {
    //     return 3;
    // }
    cells_free_cpy_cell(&cl_meta_info);
    return 0;
}


/**
 * @brief Make block dirty, update it's state (free space left) and  update 
 * block's corresponding meta-info in `block_header`
 * 
 * @param bl_info 
 * @param found_block 
 * @param new_block_state 
 * @return int 0 - Success
 * @return int 1 - Failed to get meta cell with block's state
 * @return int 2 - Failed to update meta cell with block's state
 * @return int 3 - Failed to release pinned meta cell
 */
int storage_update_block_meta_info(struct blocks_info* const bl_info, struct block* const modified_block, const struct block_state new_block_state) {
    struct cell* found_meta_cell = NULL;

    // no need then block is just created. Might decompose this to avoid redudant complexity
    blocks_set_block_state(modified_block, new_block_state);
    // Update corresponding cell_block in block_header
    if (storage_get_meta_info_of_block(bl_info, modified_block -> real_desc, &found_meta_cell) != 0) {
        return 1;
    }
    found_meta_cell -> ptr.cl_block -> free_bl_size = new_block_state.free_cells;
    if (storage_set_meta_info_of_block(bl_info,  modified_block, found_meta_cell) != 0) {
        return 2;
    }
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, found_meta_cell) != 0) {
    //     return 3;
    // }
    return 0;
}

// int storage_update_insert_queries(struct blocks_info* const bl_info, const bl_desc hint) {
//     return -1;
// }


// Operations with blocks

bl_desc storage_get_header_desc_of(const size_t bl_size, const bl_desc bld) {
    size_t bl_pos_in_group = storage_get_bl_position_in_header_group(bl_size, bld);
    return bld - bl_pos_in_group; 
}

/**
 * @brief Get the bl position in header group object
 * 
 * @param bl_size 
 * @param bld 
 * @return ssize_t 
 */
size_t storage_get_bl_position_in_header_group(const size_t bl_size, const bl_desc bld) {
    size_t header_bl_capacity = storage_get_header_group_size(bl_size);
    return bld % header_bl_capacity;
}

size_t storage_get_header_group_size(const size_t bl_size) {
    size_t bl_header_size = blocks_get_block_header_size(BLOCK_HEAD);
    size_t cell_size = cells_get_cell_type_size(CELL_BLOCK);
    return (bl_size - bl_header_size) / cell_size;
}


/**
 * @brief 
 * 
 * @param bl_info 
 * @param bld 
 * @param cell 
 * @return true 
 * @return false 
 */
bool storage_try_fit_cell_to_block(struct blocks_info* const bl_info, const bl_desc bld, const struct cell* const new_cl) {
    struct cell* cl_block_wrap = NULL;
    struct cell_block* cl_block;
    size_t cell_size = cells_get_cell_size(new_cl);
    bool ret_val;

    if (storage_get_meta_info_of_block(bl_info, bld, &cl_block_wrap) != 0) {
        // failure
        return false;
    }
    cl_block = cl_block_wrap -> ptr.cl_block;
    // Check if provided  cell can be inserted
    if (cl_block -> stor_cl_type == new_cl -> type && cl_block -> free_bl_size >= cell_size) {
        ret_val = true;
    } else {
        ret_val = false;
    }
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, cl_block_wrap) != 0) {
    //     // failure
    // }
    free(cl_block_wrap);
    return ret_val;
}


// Operations with blocks_info

int storage_get_storage_fd(const struct blocks_info* const bl_info) {
    return bl_info -> storage_fd;
}

size_t storage_get_block_size(const struct blocks_info *const bl_info) {
    return bl_info -> block_size;
}

/**
 * @brief Get last ?virtual? block descriptor in storage
 * 
 * @param bl_info 
 * @return bl_desc 
 */
bl_desc storage_get_last_bl_desc(const struct blocks_info* const bl_info) {
    GHashTableIter hash_table_iter;
    bl_desc* key_bl_desc;
    struct block* value_block = NULL;
    gboolean iter_res;

    g_hash_table_iter_init(&hash_table_iter, bl_info -> header_blocks_table);
    do {
        iter_res = g_hash_table_iter_next(&hash_table_iter, (void*) &key_bl_desc, (void*)  &value_block);
        if (iter_res == FALSE) {
            return UNDEF_BL_DESC;
        }
    } while (value_block == NULL);
    return value_block -> ptr.bl_header -> last_bl_desc;
}

/**
 * @brief Update value of the last virtual block descriptor in all of header blocks  
 * to ensure consistency  
 * @param bl_info 
 * @param new_last_bl_desc 
 */
void storage_set_last_bl_desc(const struct blocks_info* const bl_info, const bl_desc new_last_bl_desc) {
    GHashTableIter hash_table_iter;
    bl_desc* key_bl_desc;
    struct block* value_block;

    g_hash_table_iter_init(&hash_table_iter, bl_info -> header_blocks_table);
    while (g_hash_table_iter_next(&hash_table_iter, (void*) &key_bl_desc, (void*)  &value_block) == TRUE) {
        value_block -> ptr.bl_header -> last_bl_desc = new_last_bl_desc; // add set_dirty
    }
}

struct cl_desc storage_get_root_cell(const struct blocks_info* const bl_info) {
    GHashTableIter hash_table_iter;
    bl_desc* key_bl_desc;
    struct block* value_block = NULL;
    gboolean iter_res;

    g_hash_table_iter_init(&hash_table_iter, bl_info -> header_blocks_table);
    do {
        iter_res = g_hash_table_iter_next(&hash_table_iter, (void*) &key_bl_desc, (void*)  &value_block);
        if (iter_res == FALSE) {
            return cells_get_default_cld();
        }
    }
    while (value_block != NULL);
    return value_block -> ptr.bl_header -> root_cl_desc;
}

void storage_set_root_cell(const struct blocks_info* const bl_info, const struct cl_desc root_cl_desc) {
    GHashTableIter hash_table_iter;
    bl_desc* key_bl_desc;
    struct block* value_block;

    g_hash_table_iter_init(&hash_table_iter, bl_info -> header_blocks_table);
    while (g_hash_table_iter_next(&hash_table_iter, (void*) &key_bl_desc, (void*)  &value_block) == TRUE) {
        value_block -> ptr.bl_header -> root_cl_desc = root_cl_desc; 
    }
}



bl_desc storage_use_free_bl_desc(const struct blocks_info* const bl_info) {
    bl_desc free_bld;
    bl_desc* pop_bld_ptr;
    pop_bld_ptr = (bl_desc*) g_queue_pop_head(bl_info -> free_blocks);
    if (pop_bld_ptr == NULL) {
        return UNDEF_BL_DESC;
    }
    free_bld = *pop_bld_ptr;
    free(pop_bld_ptr);
    return free_bld;
}

void storage_append_free_bl_desc(struct blocks_info* const bl_info, const bl_desc free_bl_desc) {
    bl_desc* free_bld_ptr = malloc(sizeof(bl_desc));
    if (free_bld_ptr == NULL) {
        // return 1;
    }
    *free_bld_ptr = free_bl_desc;
    g_queue_push_tail(bl_info -> free_blocks, free_bld_ptr);
}


bl_desc storage_append_desc_of_loaded_bl(struct blocks_info* const bl_info, const struct block* const appending_bl) {
    GHashTable* bl_hash_table = NULL;
    bool is_bl_inserted = false;

     if (appending_bl == NULL) {
        return UNDEF_BL_DESC;
    }

    switch (appending_bl -> type) {
        case BLOCK_HEAD: {
            bl_hash_table = bl_info -> header_blocks_table;
            break;
        };
        case BLOCK_META: {
            // Todo: add insertion to loaded_meta_blocks
            bl_hash_table = bl_info -> loaded_blocks_table;
            break;
        };
        case BLOCK_DATA_FIX:
        case BLOCK_DATA_DYN: {
            bl_hash_table = bl_info -> loaded_blocks_table;
            break;
        };
        default: return UNDEF_BL_DESC;
    }
    // Use desc from structure to avoid malloc
    is_bl_inserted = g_hash_table_insert(bl_hash_table, (void*) &(appending_bl -> real_desc), (void*) appending_bl);
    // Todo: mb return v_bl_desc of inserted block?
    return is_bl_inserted ? appending_bl -> real_desc : UNDEF_BL_DESC;
}

bl_desc storage_remove_desc_of_loaded_bl(struct blocks_info* const bl_info, const struct block* const removing_bl) {
    GHashTable* bl_hash_table = NULL;
    bool is_bl_removed = false;

    if (removing_bl == NULL) {
        return UNDEF_BL_DESC;
    }

    switch (removing_bl -> type) {
        case BLOCK_HEAD: {
            bl_hash_table = bl_info -> header_blocks_table;
            break;
        };
        case BLOCK_META: {
            // Todo: add insertion to loaded_meta_blocks
            bl_hash_table = bl_info -> loaded_blocks_table;
            break;
        };
        case BLOCK_DATA_FIX:
        case BLOCK_DATA_DYN: {
            bl_hash_table = bl_info -> loaded_blocks_table;
            break;
        };
        default: return UNDEF_BL_DESC;
    }

    is_bl_removed = g_hash_table_remove(bl_hash_table, (void*) &(removing_bl -> real_desc));
    return is_bl_removed ? removing_bl -> real_desc : UNDEF_BL_DESC;
}
