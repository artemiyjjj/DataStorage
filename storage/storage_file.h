#ifndef STORAGE_FILE_H
#define STORAGE_FILE_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int open_file(const char* filename, const bool create, int* fd);

/* Extend storage file by 1 page size (4096 bytes by default)
or reduce it to the size of pages set by file_offset param

Params:
new_storage_size - size to truncate file to.
*/
int trunc_file(const int fd, const off_t new_storage_size);

int load_file_region(const int fd, const off_t file_offset, size_t mmap_size, void** mmap_addr);

int store_file_region(void** mmap_addr, size_t mmap_size);

int remove_file_region(void** mmap_addr, size_t mmap_size);

#endif // STORAGE_FILE_H
