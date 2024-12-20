#ifndef STORAGE_H
#define STORAGE_H

#include "blocks/block_info_pub.h"
#include "utils/iterators/iterators.h"


int init_storage(const char* filename, struct blocks_info** bl_info);

void close_storage(struct blocks_info** bl_info);

iterator* exec_query(struct blocks_info* const, iterator* qIt);


#endif // STORAGE_H
