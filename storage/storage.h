#ifndef STORAGE_H
#define STORAGE_H

#include "storage_file.h"
#include "../blocks/blocks.h"

// change fd to some struct for storage and block management

int init_storage(const char* filename, int* fd);

int close_storage(int* fd);

#endif // STORAGE_H
