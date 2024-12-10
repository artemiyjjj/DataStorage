#include "blocks_pub.h"

#include "block_types.h"
#include "cells/cell_types_pub.h"


enum bl_type blocks_get_block_type(const struct block* const block) {
    return block -> type;
}

bool blocks_get_block_dirty(const struct block* const block) {
    return block -> is_dirty;
}

void blocks_set_block_dirty(struct block* const block, const bool is_dirty) {
    block -> is_dirty = is_dirty;
}

struct block_state blocks_get_block_state(const struct block* const block) {
    switch (block -> type) {
        case BLOCK_HEAD:     return block -> ptr.bl_header -> state;
        case BLOCK_DATA_FIX: return block -> ptr.bl_data_fix -> state;
        case BLOCK_DATA_DYN: return block -> ptr.bl_data_dyn -> state;
        case BLOCK_META:     return block -> ptr.bl_meta -> state;
        default:             return (struct block_state) {0};
    }
}

void blocks_set_block_state(struct block* const block, const struct block_state block_state) {
    blocks_set_block_dirty(block, true);
    switch (block -> type) {
        case BLOCK_HEAD:     {
            block -> ptr.bl_header -> state = block_state;
            break;
        };
        case BLOCK_DATA_FIX: {
            block -> ptr.bl_data_fix -> state = block_state;
            break;
        };
        case BLOCK_DATA_DYN: {
            block -> ptr.bl_data_dyn -> state = block_state;
            break;
        };
        case BLOCK_META:     {
            block -> ptr.bl_meta -> state = block_state;
            break;
        }
        default: return;
    }
}


enum cl_type blocks_get_block_data_type(const struct block* const bl) {
    switch (bl -> type) {
        case BLOCK_HEAD:     return bl -> ptr.bl_header -> data_type;
        case BLOCK_DATA_FIX: return bl -> ptr.bl_data_fix -> data_type;
        case BLOCK_DATA_DYN: return bl -> ptr.bl_data_dyn -> data_type;
        case BLOCK_META:     return bl -> ptr.bl_meta -> data_type;
        default: return CELL_UNDEF;
    }
}

/**
 * @brief Get pointer to the start of the contents (cell space)
 * of the provided block.
 *
 * Pointer to char is returned to be able to perform pointer arithmetics
 * with byte-length steps.
 * 
 * @param bl - pointer to a block loaded in-memory, which contents should be returned
 * @return char* - pointer to the start of the block's contents
 */
char* blocks_get_block_contents_start(const struct block* const bl) {
    return ((char*) bl -> ptr.common) + blocks_get_block_header_size(bl -> type);
}

bl_desc blocks_get_real_bl_desc(const struct block* const bl) {
    return bl -> real_desc;
}
