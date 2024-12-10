#ifndef BLOCK_INFO_PUB_H
#define BLOCK_INFO_PUB_H

#include "block_types_pub.h"

#include "blocks/block_types.h"
#include "cells/cell_types_pub.h"


#define BLOCK_MMAP_SIZE 4096 // remove, change to posix_stat() in `storage_new_blocks_info` ...

#define INSERT_CAND_QUEUE_LEN 20

enum cell_operation {
    CL_INSERT = 1,
    CL_UPDATE,
    // CL_SELECT,
    CL_DELETE
};

struct blocks_info;

struct blocks_info* storage_new_blocks_info(void);

void storage_destroy_blocks_info(struct blocks_info** const bl_info);


int storage_find_new_cell_place(struct blocks_info* const, const bl_desc hint, const struct cell* const new_cl, struct cell** const found_cl);

int storage_update_block_meta_info_by_cell_operation(struct blocks_info* const, const struct cell* const modifying_cell, const enum cell_operation);

int storage_load_block(struct blocks_info* const, const bl_desc, struct block** const loaded_block);

int storage_pin_block(struct blocks_info* const, const struct cl_desc, const enum cl_pin_mode pin_mode);

int storage_unpin_block(struct blocks_info* const, const struct cl_desc, const enum cl_pin_mode pin_mode);

void storage_try_append_insertion_candidates(struct blocks_info* const, const struct block* const insertion_candidate_bl);

#endif
