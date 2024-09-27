#include "storage.h"

#include <stdlib.h>
#include <assert.h>

/* Storage creates, initializes, closes storage file and also serves
as an interface between queries and blocks & cells layers.
*/


static int init_blocks_info(struct blocks_info** bl_info) {
    *bl_info = malloc(sizeof(struct blocks_info)); // check
    if (bl_info == NULL) {
        return 1;
    }
    *(*bl_info) = (struct blocks_info) {
        .storage_fd = -1,
        .block_size = MMAP_SIZE,
        .header_blocks_list = NULL,
        .loaded_blocks_list = NULL,
        .get_last_bl_desc = get_last_bl_desc,
        .set_last_bl_desc = set_last_bl_desc,
        .get_root_cell = get_root_cell,
        .set_root_cell = set_root_cell,
        // ...
    };
    return 0;
}

static int create_open_storage(const char* const filename,
                               struct blocks_info* const bl_info) {
    struct block* block;
    if (open_file(filename, true, &(bl_info -> storage_fd)) != 0) {
        return 1;
    }
    // Create header block for storage meta info
    if (create_block(bl_info, 0, BLOCK_HEAD, &block) != 0) {
        close(bl_info -> storage_fd);
        return 3;
    }
    return 0;
}

static int load_header_blocks(struct blocks_info* const bl_info) {
    struct block* header_block = NULL;
    struct block_list* head_list = bl_info -> header_blocks_list;
    bl_desc cur_header_bd = 0;
    do {
        if (load_block(bl_info, cur_header_bd, true, &header_block) != 0) {
            return 1;
        }
        assert(header_block != NULL);
        assert(header_block -> type == BLOCK_HEAD);
        cur_header_bd = header_block -> ptr.bl_header -> next_header_block_desc;
    } while (cur_header_bd != UNDEF_BL_DESC);
    return 0;
}

/* Header blocks can be removed only from here */
static int remove_header_blocks(struct blocks_info* const bl_info) {
    struct block_list* header_bl_list = bl_info -> header_blocks_list;
    struct block_list* next_header_bl_list = NULL;
    bl_desc header_bl_desc = UNDEF_BL_DESC;

    while (header_bl_list != NULL) {
        next_header_bl_list = header_bl_list -> next;
        header_bl_desc = header_bl_list -> block -> desc;
        if (remove_block(bl_info, header_bl_desc, true) != 0) {
            return 1;
        }
        header_bl_list = next_header_bl_list;
    }
    return 0;
}

int init_storage(const char* filename, struct blocks_info** bl_info) {
    if (init_blocks_info(bl_info) != 0) {
        return 1;
    }

    if (open_file(filename, false, &((*bl_info) -> storage_fd)) != 0) {
         fprintf(stderr, "Failed to open storage with filename \"%s\".\n", filename);
        if (create_open_storage(filename, *bl_info) != 0) {
            fprintf(stderr, "Failed to create storage.");
            return 2;
        }
    }
    fprintf(stdout, "Storage \"%s\" opened.\n", filename);
    if (load_header_blocks(*bl_info) != 0) {
        close_storage(bl_info);
        return 3;
    }
    return 0;
}

int close_storage(struct blocks_info** bl_info) {
    struct block_list* loaded_bl_list = NULL;

    // Change for hashset behaviour!!!
    while (loaded_bl_list = (*bl_info) -> loaded_blocks_list, loaded_bl_list != NULL) {
        remove_block(*bl_info, loaded_bl_list -> block -> desc, false);
    }
    // free other lists and hashmaps!!!

    remove_header_blocks(*bl_info);
    close((*bl_info) -> storage_fd);
    free(*bl_info);
    return 0;
}


int insert() {
    return 1;
}
