#ifndef CELL_TYPES_H
#define CELL_TYPES_H

#include "blocks/block_types.h"

struct cl_desc {
    bl_desc bl_d;
    bl_offset bl_off;
};

#define UNDEF_CL_SIZE 0

enum cl_type {
    CELL_INT32 = 0,
    CELL_FLOAT32,
    CELL_BOOL,
    CELL_STRING,
    CELL_BLOCK,
    CELL_META,
};

struct cell_block {
    enum cl_type ct;

};

struct cell_meta {
    enum cl_type ct;
    
};

struct cell_int32 {
    enum cl_type ct;

};

struct cell_float32 {
    enum cl_type ct;

};

struct cell_bool {
    enum cl_type ct;
    
};

struct cell_string {
    enum cl_type ct;

};

#endif