#include "include/storage_file.h"

#define MMAP_SIZE 4096

/*
 * mode - true for new file; false for existing file only
*/
int open_storage(const char* filename, bool mode, int* fd) {
	int open_flags = mode ? O_CREAT | O_RDWR :  O_RDWR;
	*fd = open(filename, open_flags, S_IWUSR | S_IRUSR);
	if (*fd == -1) {
		return 1;
	}
	return 0;
}

int trunc_storage(int fd, off_t file_offset) {
	off_t new_storage_size = file_offset + MMAP_SIZE;
	if (ftruncate(fd, new_storage_size) == 0) {
		fprintf(stdout, "Storage truncated from %d to %d\n", file_offset, new_storage_size);
		return 0;
	} else {
		fprintf(stderr, "%s\n", "Failed to truncate storage");
		return 1;
	}
}

int load_block(int fd, void** mmap_addr, off_t file_offset) {
	*mmap_addr = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, file_offset);
	if (mmap_addr == MAP_FAILED) {
		fprintf(stderr, "%s\n", "Failed to map block");
		return 1;
	}
	fprintf(stdout, "%s %x\n", "Successfully maped block to", *mmap_addr);
	return 0;
}

// redo interface of writing (or remove)
int store_block(void* mmap_addr) {
	if (msync(mmap_addr, MMAP_SIZE, MS_SYNC) == 0) {
		fprintf(stdout, "Successfully synced %d bytes from %x\n", MMAP_SIZE, mmap_addr);
		return 0;
	} else {
		fprintf(stderr, "Failed to sync block");
		return 1;
	}
}

int remove_block(void* mmap_addr) {
	if (munmap(mmap_addr, MMAP_SIZE) == 0) {
		fprintf(stdout, "%s %x\n", "Successfully unmapped page at", mmap_addr);
		return 0;
	} else {
		fprintf(stderr, "%s %x\n", "Failed to unmap block at", mmap_addr);
		return 1;
	}
}

