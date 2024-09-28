#include "block_cli.h"

#define LOG_PREFIX "*** "

static void fprintf_block_type_header(FILE* log_stream, struct block_header* bl_header) {
    fprintf(log_stream, "%sroot cell: { cell_desc: %ld, cell_offset: %ld}\n", LOG_PREFIX, bl_header -> root_cl_desc.bl_d, bl_header -> root_cl_desc.bl_off);
    fprintf(log_stream, "%snext block header: %ld\n", LOG_PREFIX, bl_header -> next_header_block_desc);
    fprintf(log_stream, "%sstate - last block descriptor: %ld\n", LOG_PREFIX, bl_header -> state.last_bl_desc);
    fprintf(log_stream, "%sstate - free cells: %u\n", LOG_PREFIX, bl_header -> state.free_cells);
    fprintf(log_stream, "%scell_size: %c\n", LOG_PREFIX, bl_header -> cell_size);
}

int fprintf_block_info(FILE* log_stream, struct block* block) {
    fprintf(log_stream, "\ndesc: %ld", block -> desc);
    switch (block -> type) {
        case BLOCK_HEAD: {
            fprintf(log_stream, " --- Block header\n");
            fprintf_block_type_header(log_stream, block -> ptr.bl_header);
            break;
        }
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