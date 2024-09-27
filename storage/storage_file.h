#ifndef STORAGE_FILE_H
#define STORAGE_FILE_H

#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#define MMAP_SIZE 4096

int open_file(const char* filename, const bool create, int* fd);

/* Extend storage file by 1 page size (4096 bytes by default)
or reduce it to the size of pages set by file_offset param

Params:
new_storage_size - size to truncate file to.
*/
int trunc_file(const int fd, const off_t new_storage_size);

int load_file_region(const int fd, const off_t file_offset, void** mmap_addr);

int store_file_region(void** mmap_addr);

int remove_file_region(void** mmap_addr);

#endif // STORAGE_FILE_H
