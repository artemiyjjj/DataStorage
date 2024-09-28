#include "blocks.h"
#include "cli/block_cli.h"

#include <assert.h>
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

/* Calculate offset to the start or end of the block with
given block descriptor.
Parameters:
block_size - size of one and each block in the storage.
bl_desc - descriptor (position in storage) of a block.
start - count offset to start or to end of the block.
Return value:
Offset from the start of the storage file to the
start or end of the block (in bytes).
*/
unsigned int calc_file_offset(const unsigned int block_size, bl_desc bl_desc, const bool start) {
    return block_size * (start ? bl_desc : ++bl_desc);
}

/* Load block with the given bd to memory and assign it
to the corresponded list of blocks metainformation.
Return codes:
1 - Failed to mmap page from file.
2 - Failed to allocate memory for new block metadata.
*/
int load_block(struct blocks_info* const bl_info, const bl_desc bd, bool is_header, struct block** loaded_bl) { // , bool force
    void* loaded_block_addr = NULL;

    // Search in list of loaded data blocks
    if (get_block_from_list(bl_info, bd, (is_header ? true : false), loaded_bl) == 0) {
        return 0;
    }
    // Load if not found
    unsigned int file_offset = calc_file_offset(bl_info -> block_size, bd, true);
    if (load_file_region(bl_info -> storage_fd, file_offset, &loaded_block_addr) != 0) {
        return 1;
    }
    if (add_block_to_list(bl_info, bd, loaded_block_addr, loaded_bl) == 0) {
        return 0;
    }
    return 2;
}

static int sync_block_struct(struct block* block) {
    void** ptr = (void**) &(block -> ptr.bl_header);
    if (store_file_region((void**) &(block -> ptr.bl_header)) == 0) {
        return 0;
    }
    return 1;
}

/* Sync contents of loaded dirty block with storage
Return codes:
1 - Block with given block descriptor not found
2 - Failed to sync block with storage
*/
int sync_block(struct blocks_info* const bl_info, const bl_desc bd, bool is_header) {
    struct block* block;

    if (get_block_from_list(bl_info, bd, (is_header ? true : false), &block) != 0) {
        return 1;
    }
    if (block -> dirty == false) {
        return 0;
    }
    if (sync_block_struct(block) == 0) {
        block -> dirty = false;
        return 0;
    }
    return 1;
}

/* Sync block with storage, remove block from memory (munmap page) and from list of loaded blocks.
Return codes:
1 - 
*/
int remove_block(struct blocks_info* const bl_info, const bl_desc bd, bool is_header) { // bool sync - sync or remove unsaved
    struct block* block;

    if (get_block_from_list(bl_info, bd, (is_header ? true : false), &block) != 0) {
        return 1;
    }
    if (block -> dirty && sync_block_struct(block) != 0) {
        return 2;
    }
    if (remove_file_region((void**) &(block -> ptr)) != 0) {
        return 3;
    }
    if (remove_block_from_list(bl_info, bd, is_header ? true : false) != 0) {
        return 4;
    }
    return 0;
}

/* Delete block and from both storage and memory.
*/
int delete_block(struct blocks_info* const bl_info, bl_desc bd) {
    return 1;
}

/* Create block, write to file and add to loaded block list.
Return codes:
1 - Failed to truncate file
2 - Failed to load region
3 - Failed to create block struct
4 - Failed to allocate memory for block list ------ change for hashset!!!
5 - Failed to sync block
*/
int create_block(struct blocks_info* const bl_info, enum bl_type bt, struct block** created_bl) {
    // Update to support creating blocks from the middle of the storage
    void* loaded_block_addr = NULL;

    // Should refer to a struct with list of free blocks which have priority to use.
    // if struct have elements in list - use them; else get last_desc + 1
    // if (free_blocks_list != NULL) { ... }
    bl_desc desc = (bl_info -> get_last_bl_desc(bl_info)) + 1;


    unsigned int file_offset_start = calc_file_offset(bl_info -> block_size, desc, true);
    unsigned int file_offset_end = calc_file_offset(bl_info -> block_size, desc, false);

    if (trunc_file(bl_info -> storage_fd, file_offset_end) != 0) {
        return 1;
    }
    // Load region in memory and create block structure
    if (load_file_region(bl_info -> storage_fd, file_offset_start, &loaded_block_addr) != 0) {
        return 2;
    }
    if (create_block_struct(bl_info, bt, &loaded_block_addr) != 0) {
        return 3;
    }
    if (add_block_to_list(bl_info, desc, loaded_block_addr, created_bl) != 0) {
        return 4;
    }
    // Update info about last block
    bl_info -> set_last_bl_desc(bl_info, desc);
    fprintf_block_info(stdout, *created_bl);
    if (sync_block_struct(*created_bl) != 0) {
        return 5;
    }
    return 0;
}
