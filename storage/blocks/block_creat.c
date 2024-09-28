#include "block_creat.h"
#include "cells/cell_creat.h"

int create_block_struct(struct blocks_info* const bl_info, enum bl_type bt,
                        void** const block_addr) {
    switch (bt) {
            case BLOCK_HEAD: {
                struct block_header* new_bl_h = *block_addr;
                bl_desc new_last_bl_desc = bl_info -> get_last_bl_desc(bl_info);
                struct cl_desc cl_desc = bl_info -> get_root_cell(bl_info);
                long int cell_size = sizeof(struct cell_block);
                unsigned int free_cells = (bl_info -> block_size - sizeof(struct block_header)) / cell_size;

                *new_bl_h = (struct block_header) {
                    .type = bt,
                    .root_cl_desc = cl_desc,
                    .next_header_block_desc = UNDEF_BL_DESC,
                    .state = {
                    // last_bl_desc will be inconsistent before return to create_block()
                        .last_bl_desc = new_last_bl_desc,
                        .free_cells = free_cells, },
                    .cell_size = cell_size,
                };
                // update next_header_block in current header block - only place where its needed
                // get_last_bl_header().set_next_header_block(bd);
                break;
            };
            case BLOCK_DATA_FIX: {

                break;
            };
            case BLOCK_DATA_DYN: {

                break;
            }
            // ...
            default: {
                return 5;
            }
        }
    return 0;
}