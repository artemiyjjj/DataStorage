#include "storage_file.h"

/*
 * create - true for new file; false for existing file only
*/
int open_file(const char* filename, const bool create, int* fd) {
	int open_flags = create ? O_CREAT | O_RDWR :  O_RDWR;
	*fd = open(filename, open_flags, S_IWUSR | S_IRUSR);
	if (*fd == -1) {
		return 1;
	}
	return 0;
}

int trunc_file(const int fd, const off_t new_storage_size) {
	if (ftruncate(fd, new_storage_size) == 0) {
		fprintf(stdout, "Storage truncated to %d\n", new_storage_size);
		return 0;
	} else {
		fprintf(stderr, "%s\n", "Failed to truncate storage");
		return 1;
	}
}

int load_file_region(const int fd, const off_t file_offset, void** mmap_addr) {
	*mmap_addr = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, file_offset);
	if (mmap_addr == MAP_FAILED) {
		fprintf(stderr, "%s\n", "Failed to map block");
		return 1;
	}
	fprintf(stdout, "%s %x\n", "Successfully maped block to", *mmap_addr);
	return 0;
}

// redo interface of writing (or remove)
int store_file_region(void** mmap_addr) {
	if (msync(*mmap_addr, MMAP_SIZE, MS_SYNC) == 0) {
		fprintf(stdout, "Successfully synced %d bytes from %x\n", MMAP_SIZE, mmap_addr);
		return 0;
	} else {
		fprintf(stderr, "Failed to sync block");
		return 1;
	}
}

int remove_file_region(void** mmap_addr) {
	if (munmap(*mmap_addr, MMAP_SIZE) == 0) {
		fprintf(stdout, "%s %x\n", "Successfully unmapped page at", mmap_addr);
		return 0;
	} else {
		fprintf(stderr, "%s %x\n", "Failed to unmap block at", mmap_addr);
		return 1;
	}
}

