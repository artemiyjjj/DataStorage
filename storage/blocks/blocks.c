#include "blocks.h"

#include "block_info.h"
#include "blocks/block_info_pub.h"
#include "blocks_pub.h"
#include "block_types.h"
#include "blocks/block_types_pub.h"
#include "cells/cell_types_pub.h"
#include "storage_file.h"

#include <assert.h>
#include <glib.h>
#include <stdlib.h>


/* Block layer directly uses functions for manipulating 
storage's pages and manages block-related structures.
Functions from the layer is used by cell layer.

+-------+       +--------+       +--------------+
| cells | <---- | blocks | <---- | storage file |
+-------+       +--------+       +--------------+

Main function of block layer is to load requested blocks
from storage, store then they get updated and remove them
from memory and layer's structures then other blocks are
requested more often.
*/

/* Calculate offset to the start or to end of the block with
given "real" block descriptor.
Parameters:
block_size - size of each block in the storage.
bl_desc - "real" block descriptor (number of block in storage).
start - count offset to start or to end of the block.
Return value:
Offset from the start of the storage file to the
start or end of the block (in bytes).
*/
size_t calc_file_offset(const unsigned int block_size, const bl_desc bl_desc, const bool start) {
    return block_size * (start ? bl_desc : bl_desc + 1);
}

size_t blocks_get_block_header_size(const enum bl_type bl_type) {
    switch (bl_type) {
        case BLOCK_HEAD: return sizeof(struct block_header);
        case BLOCK_DATA_FIX: return sizeof(struct block_data_fix);
        case BLOCK_DATA_DYN: return sizeof(struct block_data_dyn);
        case BLOCK_META: return sizeof(struct block_meta);
        // ...
        default: return 0;
    }
}

static struct block* new_block_wrapper(const bl_desc rbd, const bool is_dirty, void* const loaded_block_addr) {
    enum bl_type bt;

    struct block* new_bl_wrap = malloc(sizeof(struct block));
    if (new_bl_wrap == NULL) {
        return NULL;
    }
    bt = ((struct block_header*) loaded_block_addr) -> type;
    new_bl_wrap -> type = bt;
    new_bl_wrap -> real_desc = rbd;
    // new_bl_wrap -> v_desc = ...
    new_bl_wrap -> is_dirty = is_dirty;
    new_bl_wrap -> ptr.common = loaded_block_addr;
    new_bl_wrap -> cell_pins2pin_counter = g_hash_table_new_full(g_int_hash, g_int_equal, free, free);
    if (new_bl_wrap -> cell_pins2pin_counter == NULL) {
        free(new_bl_wrap);
        return NULL;
    }
    return new_bl_wrap;
}

static void destroy_block_wrapper(struct block* const block) {
    g_hash_table_destroy(block -> cell_pins2pin_counter);
    free(block);
}

/**
 * @brief Load block with the given virtual block descriptor to memory and
 * create a block wrapper structure
 * 
 * @param bl_info 
 * @param vbd 
 * @param loaded_bl 
 * @return int 1 - Invalid block descriptor
 * @return int 2 - Failed to allocate memory for new block wrapper
 * @return int 3 - Failed to mmap page from file
 * @return int 4 - Failed to append block to storage meta-info
 */
int blocks_load_block(struct blocks_info* const bl_info, const v_bl_desc vbd, struct block** loaded_bl) {
    bl_desc rbd = UNDEF_BL_DESC;
    void* loaded_block_addr = NULL;
    size_t file_offset;
    size_t block_size = storage_get_block_size(bl_info);

    /// Add translation of virtual bd to real bd
    // rbd = get_real_block_descriptor(bl_info, vbd);
    rbd = vbd;
    
    file_offset = calc_file_offset(block_size, rbd, true);
    if (load_file_region(storage_get_storage_fd(bl_info), file_offset, block_size, &loaded_block_addr) != 0) {
        return 3;
    }
    *loaded_bl = new_block_wrapper(rbd, false, loaded_block_addr);
    if (*loaded_bl == NULL) {
        remove_file_region(loaded_block_addr, block_size);
        return 2;
    }
    if (storage_append_desc_of_loaded_bl(bl_info, *loaded_bl) == (*loaded_bl) -> real_desc) {
        return 0;
    } else {
        return 4;
    }
    
}

static int sync_block_struct(struct block* block, const size_t block_size) {
    if (store_file_region(&(block -> ptr.common), block_size) == 0) {
        return 0;
    }
    return 1;
}

/**
 * @brief Sync contents of loaded dirty block with storage if dirty
 * 
 * @param bl_info 
 * @param block 
 * @return int 0 - Success
 * @return int 1 - Failed to sync block with storage
 */
int blocks_sync_block(struct blocks_info* const bl_info, struct block* const block) {
    if (block -> is_dirty == false) {
        return 0;
    }
    if (sync_block_struct(block, storage_get_block_size(bl_info)) != 0) {
        return 1;
    }
    block -> is_dirty = false;
    return 0;
}

/**
 * @brief Sync block with storage, remove block from memory (munmap page) and from list of loaded blocks.
 * 
 * @param bl_info 
 * @param block 
 * @return int 0 - Success
 * @return int 1 - Failed to sync block
 * @return int 2 - Failed to remove block from memory
 * @return int 3 - Failed to remove block from storage meta-info
 */
int blocks_remove_block(struct blocks_info* const bl_info, struct block* const block) {
    size_t block_size = storage_get_block_size(bl_info);

    if (block -> is_dirty && sync_block_struct(block, block_size) != 0) {
        return 1;
    }
    if (remove_file_region(&(block -> ptr.common), block_size) != 0) {
        return 2;
    }
    if (storage_remove_desc_of_loaded_bl(bl_info, block) == block -> real_desc) {
        return 0;
    } else {
        return 3;
    }
}

/**
 * @brief Delete block from memory and truncate storage by one block size down
 * if provided block is the last in storage.
 *
 * Controls the process of deleting virtual block descriptors.
 * 
 * @param bl_info 
 * @param block 
 * @return int 0 - Success
 * @return int 1 - Failed to remove block from memory
 * @return int 2 - Failed to truncate storage
 */
int blocks_delete_block(struct blocks_info* const bl_info, struct block* const block) {
    size_t block_size = storage_get_block_size(bl_info);
    bl_desc bl_desc = block -> real_desc;
    bool is_block_last = (bl_desc == storage_get_last_bl_desc(bl_info));
    size_t new_file_offset = calc_file_offset(block_size, block -> real_desc, true);
    
    // todo: free virtual block descriptor
    if (remove_file_region(&(block -> ptr.common), block_size) != 0) {
        return 1;
    }
    if (is_block_last && trunc_file(storage_get_storage_fd(bl_info), new_file_offset) != 0) {
        return 2;
    }
    storage_append_free_bl_desc(bl_info, bl_desc);
    return 0;
}

static int construct_block(const struct blocks_info* const bl_info, const enum bl_type bt,
                               const enum cl_type ct, void** const block_addr) {
    size_t block_size = storage_get_block_size(bl_info);
    size_t cell_size = cells_get_cell_type_size(ct);
    bl_offset new_bl_offset = blocks_get_block_header_size(bt);
    bl_free_space free_cells = (block_size - new_bl_offset) / cell_size;
    switch (bt) {
            case BLOCK_HEAD: {
                struct block_header* new_bl_h = *block_addr;
                bl_desc new_last_bd = storage_get_last_bl_desc(bl_info);
                if (new_last_bd == UNDEF_BL_DESC) {
                    new_last_bd++;
                }
                struct cl_desc root_cl_desc = storage_get_root_cell(bl_info);

                new_bl_h -> type = bt;
                new_bl_h -> data_type = ct;
                // new_bl_h -> magic = BLOCK_HEADER_MAGIC;
                new_bl_h -> root_cl_desc = root_cl_desc;
                // last_bl_desc will be changed in `blocks_create_block` if this block is last
                new_bl_h -> last_bl_desc = new_last_bd;
                new_bl_h -> state.free_cells = free_cells;
                new_bl_h -> state.new_cell_start = new_bl_offset;
                break;
            };
            case BLOCK_DATA_FIX: {
                struct block_data_fix* new_bl_d_f = *block_addr;

                new_bl_d_f -> type = bt;
                new_bl_d_f -> data_type = ct;
                new_bl_d_f -> state.free_cells = free_cells;
                new_bl_d_f -> state.new_cell_start = new_bl_offset;
                break;
            };
            case BLOCK_DATA_DYN: {
                struct block_data_dyn* new_bl_d_d = *block_addr;
                free_cells = block_size - new_bl_offset;
                
                new_bl_d_d -> type = bt;
                new_bl_d_d -> data_type = ct;
                new_bl_d_d -> state.free_cells = free_cells;
                new_bl_d_d -> state.new_cell_start = new_bl_offset;
                break;
            };
            case BLOCK_META: {
                struct block_meta* new_bl_meta = *block_addr;

                new_bl_meta -> type = bt;
                new_bl_meta -> data_type = ct;
                new_bl_meta -> state.free_cells = free_cells;
                new_bl_meta -> state.new_cell_start = new_bl_offset;
                break;
            };
            default: {
                return 5;
            };
        }
    return 0;
}

/**
 * @brief Find a place for the block or truncate storage, create block header,
 * sync with storage file and add to loaded block list.
 * Controls process of managing real block descriptors and creating
 * new virtual block descriptors.
 * 
 * @param bl_info 
 * @param bt 
 * @param ct 
 * @param created_bl 
 * @return int 0 - Successfully created block of provided type
 * @return int 1 - Failed to truncate file
 * @return int 2 - Failed to load region
 * @return int 3 - Failed to create block struct
 * @return int 4 - Failed to sync block
 * @return int 5 - Failed to append block to storage meta-info
 * @return int 6 - Failed to update meta-info
 * @return int 7 - Failed to update cell about itself in bl header
 */
int blocks_create_block(struct blocks_info* const bl_info, const enum bl_type bt, const enum cl_type ct, struct block** created_bl) {
    // Update to support creating blocks from the middle of the storage
    bool is_truncated;
    void* loaded_block_addr = NULL;
    bl_desc new_bl_desc;
    // v_bl_desc new_bl_v_desc = UNDEF_BL_DESC;
    size_t file_offset_start;
    size_t file_offset_end;
    size_t block_size = storage_get_block_size(bl_info);
    int storage_fd = storage_get_storage_fd(bl_info);

    // Selection of place where to store block
    new_bl_desc = storage_use_free_bl_desc(bl_info);
    if (new_bl_desc != UNDEF_BL_DESC) {
        is_truncated = false;
    } else {
        new_bl_desc = storage_get_last_bl_desc(bl_info) + 1;
        is_truncated = true;
    }

    file_offset_start = calc_file_offset(block_size, new_bl_desc, true);
    file_offset_end = calc_file_offset(block_size, new_bl_desc, false);

    if (is_truncated && trunc_file(storage_fd, file_offset_end) != 0) {
        return 1;
    }

    // Load region in memory and create block structure
    if (load_file_region(storage_fd, file_offset_start, block_size, &loaded_block_addr) != 0) {
        return 2;
    }
    if (construct_block(bl_info, bt, ct, &loaded_block_addr) != 0) {
        return 3;
    }
    // Todo add with virtual blocks update
    #ifdef VIRT_BLOCK_DESC
    if (occupy_v_bl_desc(bl_info, rbd, &vbd) != 0) {
        return 6;
    }
    #endif
    // Todo: pass v_bl_desc
    *created_bl = new_block_wrapper(new_bl_desc, true, loaded_block_addr);
    if (*created_bl == NULL) {
        return 3;
    }
    if (storage_append_desc_of_loaded_bl(bl_info, *created_bl) != new_bl_desc) {
        return 5;
    }
    if (is_truncated) {
        storage_set_last_bl_desc(bl_info, new_bl_desc);
    }
    // Create cell with meta-info about new block in header block
    struct block_state bl_state = blocks_get_block_state(*created_bl);
    if (blocks_get_block_type(*created_bl) == BLOCK_HEAD) {
        bl_state.free_cells--;
        bl_state.new_cell_start += cells_get_cell_type_size(CELL_BLOCK);
    }
    if (0 != storage_update_block_meta_info(bl_info, *created_bl, bl_state)) {
        return 6;
    }
    if (blocks_sync_block(bl_info, *created_bl) != 0) {
        return 4;
    }
    storage_try_append_insertion_candidates(bl_info, *created_bl);
    return 0;
}
