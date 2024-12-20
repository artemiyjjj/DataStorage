#include "cells_pub.h"

#include "blocks/block_info_pub.h"
#include "blocks/block_types_pub.h"
#include "cells.h"
#include "cell_types.h"
#include "cells/cell_types_pub.h"
#include "utils/mem.h"

#include <assert.h>
#include <stdio.h>

/** 
 * This interfaces provide function for fetching, updating, inserting and deleting
 * data cells in blocks. These are directly used in storage layer to perform queries
 * and interract with blocks and cells meta-information.
 *
 * To performing insert, update and delete operations, a cell with desired cell
 * descriptor should be accessed via `find_cell` function and then modified or 
 * passed to desired operation function.
 */



/**
 * @brief Create a cell object with value compatible with provided cell type.
 * Provided value is copied into new cell. 
 * 
 * Provide pointer to value for simple types (INT32, FLOAT32, BOOL, STRING) 
 * and pointer to fully occupied cell object for other cell types. 
 * 
 * @param ct - type of the new cell
 * @param value_length - IF value is not fixed lenght, defines it's dynamic size contents
 * @param value - 
 * @return struct cell* - pointer to the created cell or NULL if allocation failed or `ct`
 */
struct cell* create_cell(enum cl_type ct, const unsigned int value_length, const void* const value) {
    void*        new_cell = NULL;
    size_t       cell_length = cells_get_cell_header_size(ct) + value_length;
    struct cell* new_cell_wrap = malloc(sizeof(struct cell));

    if (new_cell_wrap == NULL) {
        return NULL;
    }

    assert(value != NULL);
    // assertion to remove
    // size_t cell_data_type_size = cells_get_data_type_size(ct);
    // if (cell_data_type_size > 0) {
    //     assert (value_length == cell_data_type_size);
    // } else {
    //     assert (value_length > cell_data_type_size); // dummy check if provided non-fixed length type value is valid
    // }
    
    new_cell = malloc(cell_length);
    if (new_cell == NULL) {
        free(new_cell_wrap);
        return NULL;
    }

    new_cell_wrap -> ptr.common = new_cell;
    new_cell_wrap -> type = ct;
    new_cell_wrap -> cl_desc = cells_get_default_cld();

    switch (ct) {
        case CELL_INT32: { // Todo: when remove cl_type from cells, make it generic as CELL_BLOCK, ...
            memcpy(&(new_cell_wrap -> ptr.cl_int32 -> value), value, value_length);
            break;
        };
        case CELL_FLOAT32: {
            memcpy(&(new_cell_wrap -> ptr.cl_float32 -> value), value, value_length);
            break;
        }
        case CELL_BOOL: {
            memcpy(&(new_cell_wrap -> ptr.cl_bool -> value), value, value_length);
            break;
        };
        case CELL_STRING: {
            memcpy(&(new_cell_wrap -> ptr.cl_string -> value), value, value_length);
            new_cell_wrap -> ptr.cl_string -> length = value_length;
            break;
        };
        case CELL_BLOCK:
        case CELL_META:
        case CELL_OBJECT:
        case CELL_ATTR: {
            memcpy(new_cell_wrap -> ptr.common, value, cell_length);
            break;
        };
        default: {
            free(new_cell_wrap);
            free(new_cell);
            return NULL;
        };
    }
    return new_cell_wrap;
}

/**
 * @brief 
 * 
 * @param bl_info 
 * @param cl_desc 
 * @param found_cell 
 * @return struct cell* - cell with provided descriptor or NULL 
 */
struct cell* find_cell(struct blocks_info* const bl_info, const struct cl_desc cl_desc) {
    int cell_pin_res;
    struct cell* found_cell = NULL;

    cell_pin_res = cells_get_cell_ptr(bl_info, cl_desc, CL_PIN_WRITE, &found_cell);
    if (cell_pin_res == 1) {
        // wait somehow but now...
        fprintf(stderr, "Cell is locked! bl desc: %ld, bl offset: %d\n", cl_desc.bl_d, cl_desc.cl_d);
        return NULL;
    } else if (cell_pin_res != 0) {
        return NULL;
    }
    return found_cell;
}

/**
 * @brief Returned cell should be freed using `cells_free_cpy_cell`
 * 
 * @param bl_info 
 * @param cl_desc 
 * @param found_cell 
 * @return struct cell* - cell structure on the provided by descriptor position 
 */
struct cell* select_cell(struct blocks_info* const bl_info, const struct cl_desc cl_desc) {
    struct cell* found_cell = malloc(sizeof( struct cell));
    if (cells_get_cpy_cell(bl_info, cl_desc, &found_cell) != 0) {
        return NULL;
    }
    return found_cell;
}

/**
 * @brief Copy `inserting_cell` to found pinned cell`found_cell_place` and
 * release `found_cell_place`  
 * 
 * @param bl_info 
 * @param inserting_cell 
 * @param found_cell_place 
 * @return cl_desc - Successfully inserted new cell to this cell descriptor
 * @return cl_desc - UNDEF_CL_DESC - Failed to release pinned cell
 */
struct cl_desc insert_cell(struct blocks_info* const bl_info, const struct cell* const inserting_cell, struct cell* const found_cell_place) {
    size_t inserting_cell_size = cells_get_cell_size(inserting_cell);
    struct cl_desc inserted_cell_desc;

    memcpy(found_cell_place -> ptr.common, inserting_cell -> ptr.common, inserting_cell_size);
    
    if (storage_update_block_meta_info_by_cell_operation(bl_info, found_cell_place, CL_INSERT) != 0) {
        return cells_get_default_cld();
    }
    inserted_cell_desc = found_cell_place -> cl_desc;
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, found_cell_place) != 0) {
    //     return 1;
    // }
    free(found_cell_place);
    return inserted_cell_desc;
}

/**
 * @brief Cell to update should be accessed via `find_cell`, then it should be
 * modified and passed to this function. Cells of dynamic-size type should be 
 * copied, deleted and inserted. 
 * 
 * @param bl_info 
 * @param updated_cell 
 * @return int 0 - Successfully updated block and released provided cell
 * @return int -1 - Failed to update block's meta info, provided cell is not freed
 * @return int -2 - Failed to release provided cell
 */
struct cl_desc update_cell(struct blocks_info* const bl_info, struct cell* const updated_cell) {
    struct cl_desc updated_cell_desc = updated_cell -> cl_desc;
    int meta_info_upd_res = storage_update_block_meta_info_by_cell_operation(bl_info, updated_cell, CL_UPDATE);
    if (meta_info_upd_res != 0) {
        return cells_get_default_cld();
    }
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, updated_cell) != 0) {
    //     return 2;
    // }
    free(updated_cell);
    return updated_cell_desc;
}

/**
 * @brief 
 * 
 * @param bl_info 
 * @param deleting_cell 
 * @return int 0 - Success
 * @return int UNDEF_CL_DESC - Invalid cell type provided
 */
struct cl_desc delete_cell(struct blocks_info* const bl_info, struct cell* const deleting_cell) {
    struct cl_desc deleted_cell_desc = deleting_cell -> cl_desc; 
    size_t cell_size = cells_get_cell_size(deleting_cell);

    memset(deleting_cell -> ptr.common, 0, cell_size);
    switch (deleting_cell -> type) {
        case CELL_META: {
            deleting_cell -> ptr.cl_meta -> virt_cl_desc = UNDEF_CL_DESC;
            deleting_cell -> ptr.cl_meta -> bl_off = UNDEF_BL_OFFSET;
        }; break;
        case CELL_BLOCK: {
            deleting_cell -> ptr.cl_block -> vbd = UNDEF_BL_DESC;
            deleting_cell -> ptr.cl_block -> rbd = UNDEF_BL_DESC;
            deleting_cell -> ptr.cl_block -> stor_cl_type = CELL_UNDEF;
            deleting_cell -> ptr.cl_block -> free_bl_size = 0;
        }; break;
        case CELL_INT32:
        case CELL_FLOAT32:
        case CELL_BOOL:
        case CELL_STRING:
        case CELL_OBJECT:
        case CELL_ATTR: {
            deleting_cell -> ptr.cl_common -> type = CELL_UNDEF;
        }; break;
        case CELL_UNDEF:
        default: return cells_get_default_cld();
    }
    storage_update_block_meta_info_by_cell_operation(bl_info, deleting_cell, CL_DELETE);
    // if (cells_release_cell_ptr(bl_info, CL_PIN_WRITE, deleting_cell) != 0) {
    //     return 1;
    // }
    free(deleting_cell);
    return deleted_cell_desc;
}

