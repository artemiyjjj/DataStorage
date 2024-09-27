#ifndef STORAGE_H
#define STORAGE_H

#include "storage_file.h"
#include "blocks/blocks.h"


// mb remove? Nothing to decompose between storage_info and blocks_info
struct storage_info {
	int storage_fd;
	unsigned int block_size;
	struct blocks_info* blocks_info;
};

// change fd to some struct for storage and block management

int init_storage(const char* filename, struct blocks_info** bl_info);

int close_storage(struct blocks_info** bl_info);

#endif // STORAGE_H
