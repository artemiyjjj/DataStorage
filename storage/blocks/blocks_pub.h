#ifndef BLOCKS_PUB_H
#define BLOCKS_PUB_H

#include "block_types_pub.h"
#include "cells/cell_types_pub.h"

enum bl_type blocks_get_block_type(const struct block* const block);

bool blocks_get_block_dirty(const struct block* const block);

void blocks_set_block_dirty(struct block* const block, const bool is_dirty);

struct block_state blocks_get_block_state(const struct block* const block);

void blocks_set_block_state(struct block* const block, const struct block_state block_state);

bl_desc blocks_get_real_bl_desc(const struct block* const bl);

enum cl_type blocks_get_block_data_type(const struct block* const bl);

char* blocks_get_block_contents_start(const struct block* const bl);



#endif
