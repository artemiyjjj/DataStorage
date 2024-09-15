#ifndef STORAGE_FILE_H
#define STORAGE_FILE_H

#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


int open_storage(const char* filename, bool mode, int* fd);

/* Extend storage file by 1 page size (4096 bytes by default)
or reduce it to the size of pages set by file_offset param

Params:
file_offset - point, from where file will be truncated.
*/
int trunc_storage(int fd, off_t file_offset);

int load_block(int fd, void** mmap_addr, off_t file_offset);

int store_block(void* mmap_addr);

int remove_block(void* mmap_addr);

#endif // STORAGE_FILE_H
