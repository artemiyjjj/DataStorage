#include "cells_cli.h"
#include <stdio.h>

#define T_INT "CELL_INT32"
#define T_FLOAT "CELL_FLOAT32"
#define T_BOOL "CELL_BOOL"
#define T_STRING "CELL_STRING"
#define T_BLOCK "CELL_BLOCK"
#define T_META "CELL_META"
#define T_OBJECT "CELL_OBJECT"
#define T_ATTR "CELL_ATTR"

const char* cell_desc_f_str = "block: %ld, offset: %d\n";

void log_cell(FILE* log_stream, const struct cell* const cell) {
    cl_desc cld = cell -> cl_desc;
    fprintf(log_stream, "Cell: =================");
    fprintf(log_stream, cell_desc_f_str, cld.bl_d, cld.cl_d);
    switch (cell -> type) {
        case CELL_INT32: {
            fprintf(log_stream, "type: %s\n", T_INT);
            fprintf(log_stream, "value: %d\n",  cell -> ptr.cl_int32 -> value);
        } break;
        case CELL_FLOAT32: {
            fprintf(log_stream, "type %s\n", T_FLOAT);
            fprintf(log_stream, "value: %.3f\n", cell -> ptr.cl_float32 -> value);
        } break;
        case CELL_BOOL: {
            fprintf(log_stream, "type %s\n", T_BOOL);
            fprintf(log_stream, "value: %s", cell -> ptr.cl_bool -> value ? "true" : "false");
        } break;
        case CELL_STRING: {
            fprintf(log_stream, "type %s\n", T_STRING);
            fprintf(log_stream, "lenght: %u, value: %s\n", cell -> ptr.cl_string -> length, cell -> ptr.cl_string -> value);
        } break;
        case CELL_BLOCK: {
            fprintf(log_stream, "type %s\n", T_BLOCK);
            fprintf(log_stream, "virtual bl_desc: %ld, real bl_desc: %ld, data type: %d, free space: %d\n",
                     cell -> ptr.cl_block -> vbd, cell -> ptr.cl_block -> rbd, cell -> ptr.cl_block->stor_cl_type, cell -> ptr.cl_block -> free_bl_size);
        } break;
        case CELL_META: {
            // TODO
        } break;
        case CELL_OBJECT: {
            fprintf(log_stream, "type %s\n", T_OBJECT);
            fprintf(log_stream, "name: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> name);
            fprintf(log_stream, "value: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> value);
            fprintf(log_stream, "attributes: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> attrs);
            fprintf(log_stream, "children: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> children);
            fprintf(log_stream, "next sibling: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> next_sibling);
            fprintf(log_stream, "parent: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> parent);
        } break;
        case CELL_ATTR: {
            fprintf(log_stream, "type %s\n", T_ATTR);
            fprintf(log_stream, "key: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_attribute -> key);
            fprintf(log_stream, "value: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_attribute -> value);
            fprintf(log_stream, "next attr: ");
            fprintf(log_stream, cell_desc_f_str, cell_desc_f_str, cell -> ptr.cl_object -> next_sibling);
        } break;
        default: {
            break;
        }
    }
    fflush(log_stream);
}

void log_node(FILE* log_stream, const struct cell* const node);
