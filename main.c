#include <stdio.h>

#include "storage/storage.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv) {
	char* storage_name = "strg_sample";
	int storage_fd;
	void* block_addr = NULL;
	void* new_block_addr = NULL;

	if (init_storage(storage_name, &storage_fd) != 0) {
		return 1;
	}

	if (load_block(storage_fd, &block_addr, 0) != 0) {
		return 2;
	}

	char* msg = "URAAAAAA GOOOOOOOOOL GOIDA!\n";
	printf("Size of msg: %ld\n", strlen(msg));
	memcpy(block_addr, msg, strlen(msg) * sizeof(msg));

	write(1, block_addr, strlen(msg));

	if (store_block(block_addr) != 0) {
		return 3;
	}
	if (remove_block(block_addr) != 0) {
		return 4;
	}

	if (load_block(storage_fd, &new_block_addr, 0) != 0) {
		return 5;
	}

	write(1, new_block_addr, strlen(msg));

	if (remove_block(block_addr) != 0) {
		return 4;
	}

	close(storage_fd);
	return 0;
}
