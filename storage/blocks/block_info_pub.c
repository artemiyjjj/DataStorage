#include "block_info_pub.h"

#include "blocks.h"
#include "block_info.h"
#include "blocks_pub.h"
#include "block_types_pub.h"
#include "cells/cells_pub.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"
#include "utils/cli/block_cli.h"
#include "utils/mem.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>

static enum cl_type cl_types[] = {
    CELL_UNDEF,
    CELL_INT32,
    CELL_FLOAT32,
    CELL_BOOL,
    CELL_STRING,
    CELL_BLOCK,
    CELL_META,
    CELL_OBJECT,
    CELL_ATTR,
};

struct blocks_info* storage_new_blocks_info(void) {
    struct blocks_info* bl_info = malloc(sizeof(struct blocks_info));
    if (bl_info == NULL) {
        return NULL;
    }

    bl_info -> block_size = BLOCK_MMAP_SIZE;
    bl_info -> free_blocks = g_queue_new();
    bl_info -> header_blocks_table = g_hash_table_new(g_int64_hash, g_int64_equal);
    bl_info -> loaded_blocks_table = g_hash_table_new(g_int64_hash, g_int64_equal);
    // bl_info -> loaded_meta_blocks_table = g_hash_table_new(g_int64_hash, g_int64_equal);
    bl_info -> insert_candidates_queue_length = INSERT_CAND_QUEUE_LEN;
    bl_info -> cell_insertion_candidates_table = g_hash_table_new(g_int_hash, g_int_equal);

    // while (cell_type_iter -> move_next(cell_type_iter)) {
    //     ct = (enum cl_type)cell_type_iter -> current;
    //     queue_for_cl_type = g_queue_new();
    //     if (queue_for_cl_type == NULL) {
    //         return NULL;
    //     }
    //     g_hash_table_insert(bl_info -> cell_insertion_candidates_table, ct, queue_for_cl_type);
    // }

    for (unsigned int i = 0; i < sizeof(cl_types)/sizeof(enum cl_type); i++) {
        GQueue* queue_for_cl_type = g_queue_new();
        if (queue_for_cl_type == NULL) {
            return NULL;
        }
        g_hash_table_insert(bl_info -> cell_insertion_candidates_table, &cl_types[i], queue_for_cl_type);
    }
    return bl_info;
}

void storage_destroy_blocks_info(struct blocks_info** const bl_info) {
    GHashTableIter loaded_blocks_iter;
    GHashTableIter header_blocks_iter;
    GHashTableIter insertion_candidates_iter;
    bl_desc*       key_bl_desc = NULL;
    struct block*  value_block = NULL;
    enum cl_type*  key_cl_type = NULL;
    GQueue*        value_queue = NULL;

    g_hash_table_iter_init(&loaded_blocks_iter, (*bl_info) -> loaded_blocks_table);
    g_hash_table_iter_init(&header_blocks_iter, (*bl_info) -> header_blocks_table);
    g_hash_table_iter_init(&insertion_candidates_iter, (*bl_info) -> cell_insertion_candidates_table);

    // sync all loaded blocks and remove
    while (g_hash_table_iter_next(&loaded_blocks_iter, (void**) &key_bl_desc, (void**) &value_block) == TRUE) {
        // removes block from this hashtable
        g_hash_table_iter_remove(&loaded_blocks_iter);
        blocks_remove_block(*bl_info, value_block);
    }
    g_hash_table_destroy((*bl_info) -> loaded_blocks_table);
    // suyc all header blocks and remove
    while (g_hash_table_iter_next(&header_blocks_iter, (void**)  &key_bl_desc, (void**)  &value_block) == TRUE) {
        // removes block from this hashtable
        g_hash_table_iter_remove(&header_blocks_iter);
        blocks_remove_block(*bl_info, value_block);
    }
    g_hash_table_destroy((*bl_info) -> header_blocks_table);
    // free all insertion candidates and free full gqueries
    while (g_hash_table_iter_next(&insertion_candidates_iter, (void**) &key_cl_type, (void**) &value_queue) == TRUE) {
        g_hash_table_iter_remove(&insertion_candidates_iter);
        // strange bug here - for CELL_INT32 returned invalid pointer only when storage is newly created
        if (g_queue_get_length(value_queue) > 0) {
            g_queue_free_full(value_queue, free);
        } else {
            g_queue_free(value_queue);
        }
    }
    g_hash_table_destroy((*bl_info) -> cell_insertion_candidates_table);

    g_queue_free_full((*bl_info) -> free_blocks, free);
    free(*bl_info);
}

/**
 * @brief Request to synchronously load block with given block descriptor
 * from storage into memory and register it in block info's loaded_blocks.
 * Allows block management system to hide logic of blocks loading and displacing.
 * 
 *
 * If block is already in memory, pointer to the corresponding block structure
 * is set to `block` argument and no I/O is performed. 
 * 
 * @param bl_info 
 * @param bld 
 * @param block 
 * @return int 0 - Success, block is found in memory
 * @return int 1 - Success, block is loaded from storage
 * @return int -1 - Invalid block descriptor is provided - no block with given descriptor in storage
 */
int storage_load_block(struct blocks_info* const bl_info, const bl_desc bld, struct block** const loaded_block) {
    if (bld == UNDEF_BL_DESC || (bld !=0 && bld > storage_get_last_bl_desc(bl_info))) {
        return -1;
    }
    // try to lookup in headers table fisrt (useful for )
    *loaded_block = g_hash_table_lookup(bl_info -> header_blocks_table, &bld);
    if (*loaded_block != NULL) {
        log_block_info(stdout, *loaded_block, true);
        return 0;
    }
    *loaded_block = g_hash_table_lookup(bl_info -> loaded_blocks_table, &bld);
    if (*loaded_block != NULL) { 
        return 0;
    }
    // load from storage and adds to loaded blocks list
    if (blocks_load_block(bl_info, bld, loaded_block) != 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief 
 * 
 * @param bl_info 
 * @param pinned_cell_desc 
 * @param pin_mode 
 * @return int 0 - Successfully pinned a block and a cell
 * @return int 1 - Failed to load block
 * @return int 2 - Failed to allocate memory
 * @return int 3 - Failed to pin cell: provided cell desc is already pinned and can't be pinned with provided pin mode.
 */
int storage_pin_block(struct blocks_info* const bl_info, const struct cl_desc pinning_cld, const enum cl_pin_mode pin_mode) {
    struct block*       found_block;
    struct pin_counter* found_pin_counter;
    bl_offset           cell_offset = pinning_cld.cl_d;
    
    if (storage_load_block(bl_info, pinning_cld.bl_d, &found_block) < 0) {
        // if block haven't been loaded, load and pin globaly to keep it while finding cell
        return 1;
    }
    found_pin_counter = g_hash_table_lookup(found_block -> cell_pins2pin_counter, &cell_offset);
    if (found_pin_counter == NULL) { // create pin counter if it's not present
        found_pin_counter = malloc(sizeof(struct pin_counter));
        if (found_pin_counter == NULL) {
            return 2;
        }
        found_pin_counter -> cl_read_counter = 0;
        found_pin_counter -> cl_write_counter = 0;
        g_hash_table_insert(found_block -> cell_pins2pin_counter, &cell_offset, found_pin_counter);
    }

    if (pin_mode == CL_PIN_READ) {
        if (found_pin_counter -> cl_write_counter == 0) {
            found_pin_counter -> cl_read_counter++;
        } else {
            // todo: set this thread in some queue for it
            return 3;
        }
    }
    else if (pin_mode == CL_PIN_WRITE) {
        if (found_pin_counter -> cl_read_counter == 0 && found_pin_counter -> cl_write_counter == 0) {
            found_pin_counter -> cl_write_counter++;
        } else {
            // todo: set this thread in some queue for it
            return 3;
        }
    }
    return 0;
}

/**
 * @brief Unpin cell in loaded block.
 * Decrement pin counter for a needed pin mode.
 * 
 * @param bl_info 
 * @param released_cld 
 * @param pin_mode 
 * @return int 0 - Success
 * @return int 1 - Failed to load block
 * @return int 2 - Failed to find made pin
 */
int storage_unpin_block(struct blocks_info* const bl_info, const struct cl_desc released_cld, const enum cl_pin_mode pin_mode) {
    bl_offset cell_offset;
    struct block* found_block;
    struct pin_counter* found_pin_counter;
    
    if (storage_load_block(bl_info, released_cld.bl_d, &found_block) != 0) {
        return 1;
    }
    cell_offset = released_cld.cl_d;
    found_pin_counter = g_hash_table_lookup(found_block -> cell_pins2pin_counter, &cell_offset);
    if (found_pin_counter == NULL) {
        return 2;
    }

    if (pin_mode == CL_PIN_READ) {
        found_pin_counter -> cl_read_counter--;
    }
    else if (pin_mode == CL_PIN_WRITE) {
        found_pin_counter -> cl_write_counter--;
    }
    return 0;
}

/**
 * @brief 
 * 
 * @param bl_info 
 * @param new_cl 
 * @param found_bl 
 * @return int 0 - Success
 * @return int 1 - Failed to find block queue for needed cell type
 * @return int 2 - Failed to create a new block 
 * @return int 3 - Failed to find a place for cell in meta-info
 */
static int find_block_for_cell(struct blocks_info* const bl_info, const struct cell* const new_cl, bl_desc* const found_bld) {
    guint         queue_length;
    GQueue*       queue_blocks_fot_ct = NULL;
    struct block* found_bl = NULL;
    enum cl_type  ct = new_cl -> type;
    enum bl_type  bt = cells_get_bl_type_by_cl_type(ct);
    *found_bld = UNDEF_BL_DESC;

    queue_blocks_fot_ct = g_hash_table_lookup(bl_info -> cell_insertion_candidates_table, &ct);
    if (queue_blocks_fot_ct == NULL) { // if queue for cell type is not created yet
        return 1;
    }
    else if (g_queue_is_empty(queue_blocks_fot_ct)) {  // No blocks of this type (assume it always contains some blocks with free space)
        if (blocks_create_block(bl_info, bt, ct, &found_bl) != 0) {
            return 2;
        }
        *found_bld = found_bl -> real_desc;
    } else {
        queue_length = g_queue_get_length(queue_blocks_fot_ct);
        bl_desc *cur_bld;
        
        for (guint i = 0; i < queue_length; i++) {
            cur_bld = g_queue_peek_nth(queue_blocks_fot_ct, i);
            if (cur_bld && storage_try_fit_cell_to_block(bl_info, *cur_bld, new_cl)) {
                *found_bld = *cur_bld;
                break;
            }
        }
    }
    if (*found_bld == UNDEF_BL_DESC) { // mb too big for a cell, split it as a big cell for multiple blocks at higher level
        return 3;
    }
    return 0;
}

/**
 * @brief Finds a place for cell of requested size and type, using storage's
 * meta-information. Loads the block and pins it for write for found cell.
 *
 * Returned pointer to cell wrapper `found_cl` should be released after needed 
 * operations using `release_cell_ptr` function.
 *
 * Pinned block doesn't gets dirty while this operation, but gets dirty right
 * after update or insertion of found cell.
 *
 * Block, where cell has been inserted, and it's meta-info should be updated
 * after insertion using `storage_update_block_meta_info_by_cell`
 *
 * If valid hint provided - if block pointed by hint has suitable storing cell
 * type and enough space to fit a new cell - cell place will be reserved there.
 * Hint mechanism would help to develop data locality and reduce I/O.
 *
 * If hint shouldn't be encountered, provide constant `UNDEF_BL_DESC`.
 *
 * Without a hint, a block for placing new cell is found by checking meta-info
 * with block's free space in global storage's `blocks_info` structure - field
 * `cell_insertion_candidates_table` is responsible for keeping mapping between
 * cell type of blocks and dynamicly updating queue with block descriptors for 
 * cells of one type.
 * 
 * @param bl_info 
 * @param new_cl_type 
 * @param hint - disered block for placing a new cell or `UNDEF_BL_DESC`
 * @param new_cl - a cell wrap with inserting cell. Doesn't set to the found cell in this function
 * @param found_cl (set by function) - pointer to pinned cell where to put new cell
 * @return int 0 - Success
 * @return int 1 - Failed to find or create a block
 * @return int 2 - Failed to load found block
 * @return int 3 - Failed to 
 */
int storage_find_new_cell_place(struct blocks_info* const bl_info, const bl_desc hint, const struct cell* const new_cl, struct cell** const found_cl) {
    bl_desc             found_bl_desc;
    struct block*       found_block = NULL;
    struct block_state  found_bl_state;
    struct cl_desc      found_cl_desc;

    // Encounter hint block desc
    if (hint != UNDEF_BL_DESC && storage_try_fit_cell_to_block(bl_info, hint, new_cl)) {
        found_bl_desc = hint;
    }
    // Search for block in meta-info or create a new one
    else if (find_block_for_cell(bl_info, new_cl, &found_bl_desc) != 0) {
        return 1;
    }

    if (storage_load_block(bl_info, found_bl_desc, &found_block) != 0) {
        return 2;
    }
    // Find a place for the cell
    // Todo: add meta cells check of space to avoid cell overlapping
    found_bl_state = blocks_get_block_state(found_block);
    found_cl_desc.bl_d = found_bl_desc;
    found_cl_desc.cl_d = found_bl_state.new_cell_start;
    *found_cl = find_cell(bl_info, found_cl_desc);
    if (*found_cl == NULL) {
        return 3;
    }
    return 0;
}

/**
 * @brief Should be called in functions that actually change the values in 
 * pinned cells, e.g. `insert_cell`
 *
 * When dynamic type cell length is changed, it's position is changing too.
 * Update, when cell's place is changing should be performed by copying,
 * deleting and inserting cell. 
 * 
 * @param bl_info 
 * @param modified_block 
 * @param new_bl_state 
 * @return int 0 - Successfully updated
 * @return int 1 - Failed: no cell provided
 * @return int 2 - Failed: requested block is not loaded in memory and pinned
 * @return int 3 - Failed: modifying unsupported type of block
 * @return int 4 - Failed to update block's meta info
 */
int storage_update_block_meta_info_by_cell_operation(struct blocks_info* const bl_info, const struct cell* const modifying_cell,
                                                    const enum cell_operation cl_operation) {
    struct block*       modifying_bl = NULL;
    struct block_state  modifying_bl_state;
    ssize_t             cl_size = cells_get_cell_size(modifying_cell);

    if (modifying_cell == NULL) {
        return 1;
    }

    // Block should be loaded in memory and pinned
    if (storage_load_block(bl_info, modifying_cell -> cl_desc.bl_d, &modifying_bl) != 0) {
        return 2;
    }
    modifying_bl_state = blocks_get_block_state(modifying_bl);

    // do not change block state's if cell's the same
    if (cl_operation == CL_INSERT || cl_operation == CL_DELETE) {
        switch (modifying_bl -> type) {
            case BLOCK_HEAD:
            case BLOCK_DATA_FIX:
            case BLOCK_META: {
                if (cl_operation == CL_INSERT) {
                    modifying_bl_state.free_cells--;
                } else if (cl_operation == CL_DELETE) {
                    modifying_bl_state.free_cells++;
                }
                break;
            };
            case BLOCK_DATA_DYN: {
                // Todo: add new_cell_position calculating logic to avoid cell overlapping
                if (cl_operation == CL_INSERT) {
                    modifying_bl_state.free_cells -= cl_size;
                } else {
                    modifying_bl_state.free_cells += cl_size;
                }
                break;
            }
            default: return 3;
        }
        // Todo: add new_cell_position calculating logic to avoid cell overlapping
        if (modifying_bl_state.free_cells > 0) {
            modifying_bl_state.new_cell_start += cl_size;
        } else {
            modifying_bl_state.new_cell_start = UNDEF_BL_OFFSET;
        }
    }
    // For CL_UPDATE just make block dirty since update doesn't change cell size and block's free space count
    if (storage_update_block_meta_info(bl_info, modifying_bl, modifying_bl_state) != 0) {
        return 4;
    }
    // Todo: add update in block_meta's corresponding cell
    return 0;
}

/**
 * @brief Remove least recently used candidate
 * 
 * @param bl_info 
 * @param candidates_queue 
 */
static void displace_candidate(GQueue* const candidates_queue) {
    bl_desc* candidate_bl_desc = g_queue_pop_tail(candidates_queue);
    free(candidate_bl_desc);
}


void storage_try_append_insert_cand_by_bl_desc(struct blocks_info* const bl_info, const enum cl_type data_type, const bl_free_space bl_free_space, const bl_desc rbd) {
    size_t queue_max_size = storage_get_insert_cand_size(bl_info);
    GQueue* candidates_queue = NULL;
    GList* found_candidate = NULL;
    bl_desc* bl_desc_copy = NULL;

    if (bl_free_space == 0) {
        return;
    }

    candidates_queue = g_hash_table_lookup(bl_info -> cell_insertion_candidates_table, &data_type);
    assert(candidates_queue != NULL);
    if (g_queue_get_length(candidates_queue) >= queue_max_size) {
        // try to displace candidates with less "priority" (bigger amount of free cell space)
        displace_candidate(candidates_queue);
    }
    found_candidate = g_queue_find(candidates_queue, &rbd);
    if (found_candidate == NULL) {
        bl_desc_copy = myAllocStruct(bl_desc);
        if (bl_desc_copy == NULL) {
            return;
        }
        *bl_desc_copy = rbd; 
        // Todo: redefine push and displace order, store free space amount
        g_queue_push_head(candidates_queue, bl_desc_copy);
    } else {
        // already in queue
        return;
    }
}

void storage_try_append_insertion_candidates(struct blocks_info* const bl_info, const struct block* const insertion_candidate_bl) {
    bl_desc         candidate_bl_desc = blocks_get_real_bl_desc(insertion_candidate_bl);
    enum cl_type    bl_data_type = blocks_get_block_data_type(insertion_candidate_bl);
    bl_free_space   bl_free_space = blocks_get_block_state(insertion_candidate_bl).free_cells;
    storage_try_append_insert_cand_by_bl_desc(bl_info, bl_data_type, bl_free_space, candidate_bl_desc);
}
