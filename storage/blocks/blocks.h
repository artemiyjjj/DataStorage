#ifndef BLOCKS_H
#define BLOCKS_H

#include "block_info_pub.h"
#include "block_types.h"
#include "block_types_pub.h"


size_t calc_file_offset(const unsigned int block_size, const bl_desc bl_desc, const bool start);

int blocks_create_block(struct blocks_info* const bl_info, const enum bl_type, const enum cl_type, struct block** created_bl);

int blocks_load_block(struct blocks_info* const bl_info, const v_bl_desc vbd, struct block** loaded_bl);

int blocks_sync_block(struct blocks_info* const bl_info, struct block* const bl);

int blocks_remove_block(struct blocks_info* const bl_info, struct block* const bl);

int blocks_delete_block(struct blocks_info* const bl_info, struct block* const bl);


// Virtual block descriptor 
// #ifdef VIRT_BLOCK_DESC todo

// int occupy_v_bl_desc(struct blocks_info* const bl_info, const bl_desc new_rbd, v_bl_desc* const occupied_vbd);

// int change_real_bl_desc(struct blocks_info* const bl_info, const v_bl_desc vbd, const bl_desc new_rbd);

// int free_v_bl_desc(const v_bl_desc vbd);

#endif // BLOCKS_H
