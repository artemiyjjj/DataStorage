
#include "cells/cell_types_pub.h"
#include "storage/storage.h"
#include "utils/cli/block_cli.h"

#include <stdio.h>
#include <assert.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv) {
	char* storage_name = "strg_sample";
	struct blocks_info* bl_info = NULL;
	// struct block* block = NULL;
	// struct block* new_block = NULL;

	if (init_storage(storage_name, &bl_info) != 0) {
		return 1;
	}

	
	// log_block_info(stdout, bl_info -> header_blocks_table, true);
	
	// bl_desc data_bd = bl_info -> get_last_bl_desc(bl_info) + 1;
	// create_block(bl_info, BLOCK_DATA_FIX, CELL_INT32, &new_block);
	printf("before close");
	close_storage(&bl_info);
	return 0;
}
