#ifndef BLOCKS_H
#define BLOCKS_H

#include "block_info.h"
#include "storage_file.h"

unsigned int calc_file_offset(const unsigned int block_size, bl_desc bl_desc, const bool start);

int create_block(struct blocks_info* const bl_info, const bl_desc bd, const enum bl_type bt, struct block** created_bl);

int load_block(struct blocks_info* const bl_info, const bl_desc bd, const bool is_header, struct block** loaded_bl);

int sync_block(struct blocks_info* const bl_info, const bl_desc bd, bool is_header);

int remove_block(struct blocks_info* const bl_info, const bl_desc bd, bool is_header);

int delete_block(struct blocks_info* const bl_info, const bl_desc bd);

#endif // BLOCKS_H
