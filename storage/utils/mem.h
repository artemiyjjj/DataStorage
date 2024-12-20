#ifndef MEM_H
#define MEM_H

#include <stdlib.h>

#define myAllocStruct(T) ((T*)malloc(sizeof(T)))

#define myAllocArray(T, n) ((T*)malloc(sizeof(T) * n))

#endif
