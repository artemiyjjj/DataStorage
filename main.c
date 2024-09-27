#include <stdio.h>
#include <assert.h>
#include "storage/storage.h"
#include "cli/block_cli.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv) {
	char* storage_name = "strg_sample";
	struct blocks_info* bl_info = NULL;
	struct block* block = NULL;
	struct block* new_block = NULL;

	if (init_storage(storage_name, &bl_info) != 0) {
		return 1;
	}
	close_storage(&bl_info);
	return 0;
}
