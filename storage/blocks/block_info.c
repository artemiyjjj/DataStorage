#include "block_info.h"
#include "block_creat.h"

#include <assert.h>
#include <stdlib.h>

// Operations with blocks_info

bl_desc get_last_bl_desc(const struct blocks_info* const bl_info) {
    if (bl_info -> header_blocks_list != NULL) {
        return bl_info -> header_blocks_list -> block -> ptr.bl_header -> state.last_bl_desc;
    }
    return UNDEF_BL_DESC;
}

void set_last_bl_desc(const struct blocks_info* const bl_info, const bl_desc new_last_bl_desc) {
    struct block_list* header_bl_list = bl_info -> header_blocks_list;
    while (header_bl_list != NULL) {
        header_bl_list -> block -> ptr.bl_header -> state.last_bl_desc = new_last_bl_desc;
        header_bl_list = header_bl_list -> next;
    }
}

struct cl_desc get_root_cell(const struct blocks_info* const bl_info) {
    if (bl_info -> header_blocks_list == NULL) {
        return (struct cl_desc) {
            .bl_d = UNDEF_BL_DESC,
            .bl_off = UNDEF_BL_OFFSET
        };
    }
    return bl_info -> header_blocks_list -> block -> ptr.bl_header -> root_cl_desc;
}

void set_root_cell(const struct blocks_info* const bl_info, const struct cl_desc root_cl_desc) {
    /// to do
}


// Operations with block list

/* Add block to list of blocks with the same logical functionality
(header blocks to header_blocks_list, e.t.c).
Return codes:
1 - allocation of block has failed
*/
int add_block_to_list(struct blocks_info* const bl_info, const bl_desc bd,
                      void* const block_addr, struct block** block) {
    // Allocate block struct
    *block = malloc(sizeof(struct block));
    if (*block == NULL) {
        return 1;
    }
    **block = (struct block) {
        .desc = bd,
        .dirty = false,
        .ptr = block_addr
    };
    (*block) -> type = (*block) -> ptr.bl_header -> type;
    

    // Change for hashset behaviour
    struct block_list* bl_list = (*block) -> type == BLOCK_HEAD ?
                                 bl_info -> header_blocks_list : bl_info -> loaded_blocks_list;

    struct block_list* bl_list_prev = NULL;
    while (bl_list != NULL) {
        bl_list_prev = bl_list;
        bl_list = bl_list -> next;
    }
    bl_list = malloc(sizeof(struct block_list));
    if (bl_list == NULL) {
        return 1;
    }
    *bl_list = (struct block_list) {
        .block = *block,
        .next = NULL,
        .prev = bl_list_prev
    };
    if (bl_list_prev != NULL) {
        bl_list_prev -> next = bl_list;
    } else {
        bl_info -> header_blocks_list = bl_list;
    }

    return 0;
}

int get_block_from_list(const struct blocks_info* const bl_info, const bl_desc bd,
                        const bool is_header, struct block** block) {
    struct block_list* bl_list = is_header ? bl_info -> header_blocks_list : bl_info -> loaded_blocks_list;
    while (bl_list != NULL) {
        assert(bl_list -> block != NULL); //debug
        if (bl_list -> block -> desc == bd) {
            *block = bl_list -> block;
            return 0;
        }
        bl_list = bl_list -> next;
    }
    return 1;
}

/*
Return codes:
1 - Haven't found block with given descriptor in list
*/
int remove_block_from_list(struct blocks_info* const bl_info, const bl_desc bd, bool is_header) {
    // GHashTable* table = bl_info -> is_header ? header_blocks_table : loaded_blocks_table;

    // // if (!g_hash_table_contains(bd)) {
    // //     return 1;
    // // }
    // if (g_hash_table_remove(table, bd)) {
        
    // }


    // Change for hashset behaviour
    struct block_list* bl_list = is_header ? bl_info -> header_blocks_list : bl_info -> loaded_blocks_list;

    while (bl_list != NULL) {
        assert(bl_list -> block != NULL);
        // Find required block
        if (bl_list -> block -> desc == bd) {
            free(bl_list -> block);
            struct block_list* prev_bl = bl_list -> prev;
            struct block_list* next_bl = bl_list -> next;
            // Remove block_list from list chain 
            if (prev_bl != NULL) {
                prev_bl -> next = next_bl;
            }
            if (next_bl != NULL) {
                next_bl -> prev = prev_bl;
            }
            free(bl_list);
            return 0;
        }
        bl_list = bl_list -> next;
    }
    return 1;
}

