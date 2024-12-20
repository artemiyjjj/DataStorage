#include "cell_types.h"
#include "blocks/block_types_pub.h"
#include "cells/cell_types_pub.h"
#include "utils/mem.h"


cells_simple_values cells_get_simple_type_value(struct cell* cell) {
    cells_simple_values value = {0};
    switch (cell -> type) {
        case CELL_INT32: {
            value.int32 = cell -> ptr.cl_int32 -> value;
        }; break;
        case CELL_FLOAT32: {
            value.float32 = cell -> ptr.cl_float32 -> value;
        }; break;
        case CELL_BOOL: {
            value.boolean = cell -> ptr.cl_bool -> value;
        }; break;
        case CELL_STRING: {
            value.string = myAllocArray(char, cell -> ptr.cl_string -> length + 1);
            strncpy(value.string, cell -> ptr.cl_string -> value, cell -> ptr.cl_string -> length);
            value.string[cell -> ptr.cl_string -> length] = '\0';
        }; break;
        default: break;
    }
    return value;
}


struct cl_desc cells_get_default_cld(void) {
    return (struct cl_desc) { .bl_d = UNDEF_BL_DESC, .cl_d = UNDEF_CL_DESC };
}

/**
 * @brief 
 * 
 * @param first 
 * @param second 
 * @return int -1 - first has less bl_desc or equal bl_desc and less cl_desc
 * @return int 0 - descriptors are equal
 * @return int 1 - first has greater bl_desc or equal bl_desc and less cl_desc
 */
int cells_cmp_cl_desc(const cl_desc first, const cl_desc second) {
    if (first.bl_d < second.bl_d) {
        return -1;
    } else if (first.bl_d > second.bl_d) {
        return 1;
    } else if (first.cl_d < second.cl_d) {
        return -1;
    } else if (first.cl_d > second.cl_d) {
        return 1;
    } else {
        return 0;
    }
}

int cells_cmp(const struct cell* const first, const struct cell* const second) {
    if (first == NULL) {
        if (second == NULL) {
            return 0;
        }
        return -1;
    } else if (second == NULL) {
        return 1;
    }
    return cells_cmp_cl_desc(first->cl_desc, second->cl_desc);
}

/**
 * @brief Get size of a given type cell header. 
 * 
 * @param ct 
 * @return size_t 
 */
size_t cells_get_cell_header_size(enum cl_type ct) {
    switch (ct) {
        case (CELL_INT32): {
            return sizeof(struct cell_int32);
        };
        case (CELL_FLOAT32): {
            return sizeof(struct cell_float32);
        };
        case (CELL_BOOL): {
            return sizeof(struct cell_bool);
        };
        case (CELL_STRING): {
            return sizeof(struct cell_string);
        };
        case (CELL_BLOCK): {
            return sizeof(struct cell_block);
        };
        case (CELL_META): {
            return sizeof(struct cell_meta);
        };
        case (CELL_OBJECT): {
            return sizeof(struct cell_object);
        };
        case (CELL_ATTR): {
            return sizeof(struct cell_attr);
        }
        default: {
            return 0;
        };
    }
}

size_t cells_get_data_type_size(const enum cl_type ct) {
    switch (ct) {
        case (CELL_INT32): {
            return sizeof(int);
        };
        case (CELL_FLOAT32): {
            return sizeof(float);
        };
        case (CELL_BOOL): {
            return sizeof(bool);
        };
        case (CELL_STRING): {
            return 0;
        };
        case CELL_META:
        case CELL_BLOCK:
        case CELL_ATTR:
        case CELL_OBJECT:
        // case (...) {}
        default: {
            return 0;
        }
    }
}

/**
 * @brief Calculate size of the cell. Size of data corresponds to the
 * actual data size, if cell is fixed-length. If cell of given type is 
 * of non-fixed size type, when size of header is returned.
 *
 * @param ct 
 * @return size_t 
 */
size_t cells_get_cell_type_size(const enum cl_type ct) {
    size_t data_type_size = cells_get_data_type_size(ct);
    return cells_get_cell_header_size(ct) + data_type_size;
}

/**
 * @brief Calculate actual size of the cell. Useful for non-static size
 * types (such as STRING), where length of the cell can not be determined
 * by it's type.
 * 
 * @param cell 
 * @return size_t size of the cell in bytes
 */
size_t cells_get_cell_size(const struct cell* const cell) {
    size_t data_type_size = cells_get_data_type_size(cell -> type);
    size_t cell_header_size = cells_get_cell_header_size(cell -> type);
    switch (cell -> type) {
        case (CELL_INT32):
        case (CELL_FLOAT32):
        case (CELL_BOOL):
        case CELL_OBJECT:
        case CELL_ATTR:
        case (CELL_BLOCK):
        case (CELL_META): {
            return cell_header_size + data_type_size;
        };
        case (CELL_STRING): {
            return sizeof(struct cell_string) + cell -> ptr.cl_string -> length;
        };
        // case (...) {}
        default: {
            return 0;
        };
    }
}

enum cl_type cells_get_cell_type(const struct cell* const cell) {
    return cell -> type;
}

cl_desc cells_get_cell_desc(const struct cell* const cell) {
    return cell -> cl_desc;
}

/**
 * @brief Calculate offset of cell in a block by it's size assuming that cells
 * are positioned sequentially and have equal size. 
 * `BEWARE! Won't work properly with non-static length cells!`
 *
 * @param ct 
 * @param cl_index 
 * @return bl_offset 
 */
bl_offset cells_get_cell_offset_by_index(const enum cl_type ct, const size_t cl_index) {
    size_t cl_size = cells_get_cell_type_size(ct);
    return cl_size * cl_index;
}

enum bl_type cells_get_bl_type_by_cl_type(const enum cl_type cl_type) {
    switch (cl_type) {
        case CELL_STRING: return BLOCK_DATA_DYN;
        case CELL_INT32:
        case CELL_FLOAT32:
        case CELL_BOOL:
        case CELL_BLOCK:
        case CELL_META:
        case CELL_OBJECT:
        case CELL_ATTR: return BLOCK_DATA_FIX;
        default: return BLOCK_UNDEF;
    }
}
