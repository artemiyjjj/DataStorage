#include "block_cli.h"
#include "blocks/block_types.h"
#include "blocks/block_types_pub.h"
#include "blocks/blocks_pub.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"
#include <stdio.h>

#define LOG_PREFIX "*** "

static void log_cell_layout(FILE* log_stream, struct block* block) {
    enum cl_type ct = blocks_get_block_data_type(block);
    size_t cell_size = cells_get_cell_type_size(ct);
    size_t cells_amount = (4096 - blocks_get_block_header_size(block -> type)) / cell_size;
    void* bl_contents = blocks_get_block_contents_start(block);
    struct cell_block* cur_cl = NULL;
    bl_offset cur_bl_off = 0;

    for (size_t i = 0; i < cells_amount; i++) {
        cur_bl_off = i * cell_size;
        cur_cl = (struct cell_block*) (bl_contents + cur_bl_off);
        if (cur_cl -> stor_cl_type != CELL_UNDEF) {
            fprintf(log_stream, "Offset: %d, vbd: %ld, rbd: %ld, ct: %d", cur_bl_off, cur_cl -> vbd, cur_cl -> rbd, cur_cl -> stor_cl_type);
        }
        fflush(stdout);
    }
    return;
}

static void log_block_type_header(FILE* log_stream, struct block* bl_header) {
    fprintf(log_stream, "%sroot cell: { block_virtual_desc: %ld, cell_id: %hu}\n", LOG_PREFIX, bl_header->ptr.bl_header -> root_cl_desc.bl_d, bl_header ->ptr.bl_header-> root_cl_desc.cl_d);\
    fprintf(log_stream, "%sstate - last block descriptor: %ld\n", LOG_PREFIX, bl_header ->ptr.bl_header-> last_bl_desc);
    fprintf(log_stream, "%sstate - free cells: %u\n", LOG_PREFIX, bl_header ->ptr.bl_header -> state.free_cells);
    fprintf(log_stream, "%sstate - new cell offset %u\n", LOG_PREFIX, bl_header ->ptr.bl_header-> state.new_cell_start);
    fprintf(log_stream, "%scell_size: %li\n", LOG_PREFIX, cells_get_cell_type_size(bl_header ->ptr.bl_header-> data_type));
    log_cell_layout(log_stream, bl_header);
}

static void log_block_type_meta(FILE* log_stream, struct block_meta* bl_meta) {
    
}

static void log_block_type_data_fix(FILE* log_stream, struct block_data_fix* bl_data) {

}

static void log_block_type_data_dyn(FILE* log_stream, struct block_data_dyn* bl_data) {
    
}

int log_block_info(FILE* log_stream, struct block* block, bool show_cell_layout) {
    return 0;
    fprintf(log_stream, "\ndesc: %ld", block -> v_desc);
    switch (block -> type) {
        case BLOCK_HEAD: {
            fprintf(log_stream, " --- Block header\n");
            log_block_type_header(log_stream, block);
            if (show_cell_layout) { log_cell_layout(log_stream, block); }
            break;
        };
        case BLOCK_META: {
            fprintf(log_stream, " --- Block meta\n");
            log_block_type_meta(log_stream, block -> ptr.bl_meta);
            if (show_cell_layout) { log_cell_layout(log_stream, block); }
            break;
        };
        case BLOCK_DATA_FIX: {
            fprintf(log_stream, " --- Block data with fixed length cells\n");
            log_block_type_data_fix(log_stream, block -> ptr.bl_data_fix);
            if (show_cell_layout) { log_cell_layout(log_stream, block); }
            break;
        };
        case BLOCK_DATA_DYN: {
            fprintf(log_stream, " --- Block data with non fixed length cells\n");
            log_block_type_data_dyn(log_stream, block -> ptr.bl_data_dyn);
            if (show_cell_layout) { log_cell_layout(log_stream, block); }
            break;
        };
        // case ... {
        //     fpintf(" --- Block ...\n");
        //     break;
        // }

        default: {
            // error msg
            return 1;
        }
    }
    return 0;
}
