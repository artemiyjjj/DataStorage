#include "cells.h"

#include "blocks/block_info.h"
#include "blocks/block_info_pub.h"
#include "blocks/blocks_pub.h"
#include "blocks/block_types_pub.h"
#include "cells/cell_types.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/** All CRUD operations require parameter `iterator` with some kind of 
 * data which can be used to access cells fitting given conditions. 
 * 
 * There are two possible ways of implementing this and in each case 
 * iterator should return cell descriptions or `struct cell` with pointer 
 * to actual cell in a loaded and pinned block.
 * Fisrt case seems to add more separation between quering and doing the
 * query-dependent action (like update) but leads to enlarge time consumption
 * of each query and pootential amount of I/O with need of accessing each cell
 * twice (firstly then access it for checking the conditions and secondly for 
 * actual fetchig) and redudant complexity.
 */




/**
 * @brief Get copy of the cell contents with provided cell
 * descriptor without ability to modify the actual cell data. 
 * 
 * @param bl_info
 * @param cld 
 * @param cell 
 * @return int 0 - Success
 * @return int 1 - Failed to allocate memory
 * @return int 2 - Failed to access cell
 * @return int 3 - Invalid cell contents
 */
int cells_get_cpy_cell(struct blocks_info* const bl_info, const struct cl_desc searched_cld, struct cell** const cell_cpy) {
    size_t          cell_size;
    int             get_cl_ptr_res;
    void*           found_cell;
    struct cell*    found_cell_wrap = NULL;

    *cell_cpy = malloc(sizeof(struct cell));
    if (*cell_cpy == NULL) {
        return 1;
    }

    get_cl_ptr_res = cells_get_cell_ptr(bl_info, searched_cld, CL_PIN_READ, &found_cell_wrap);
    if (get_cl_ptr_res != 0) {
        return 2;
    }
    // copy cell
    cell_size = cells_get_cell_size(found_cell_wrap);
    if (cell_size >= storage_get_block_size(bl_info)) { // for cell string mostly
        return 3;
    }
    found_cell = malloc(cell_size);
    if (found_cell == NULL) {
        return 1;
        free(*cell_cpy);
    }
    memcpy(found_cell, found_cell_wrap -> ptr.common, cell_size);
    memcpy(*cell_cpy, found_cell_wrap, sizeof(struct cell));
    (*cell_cpy) -> ptr.common = found_cell;
    
    // if (cells_release_cell_ptr(bl_info, CL_PIN_READ, *cell) != 0) {
    //     return 3;
    // }
    free(found_cell_wrap);
    return 0;
}

/**
 * @brief Use for copied and manually created cells 
 * 
 * @param cell 
 * @return int 
 */
void cells_free_cpy_cell(struct cell** cell) {
    free((*cell) -> ptr.common);
    free(*cell);
}

/**
 * @brief Get access to the cell with provided descriptor on the
 * loaded and pinned page. If cell is already pinned for reading, 
 * it can not be pinned for modifying and if it can not be pinned
 * for reading if it is pinned for modifying.
 * Returned cell should be freed and its
 * block should be unpinned with `release_cell_ptr` function.
 * 
 * @param bl_info
 * @param searched_cld
 * @param found_cell 
 * @return int 0 - Cell with given descriptor is found
 * @return int 1 - Cell can't be pinned for requested operation now
 * @return int 2 - Failed to allocate memory
 * @return int 3 - Failed to load block
 * @return int 4 - Failed to pin block
 * @return int 5 - Block with descriptor given in cell is not found (Invalid bd)
 */
int cells_get_cell_ptr(struct blocks_info* const bl_info, const struct cl_desc searched_cld, const enum cl_pin_mode pin_mode, struct cell** const found_cell_wrap) {
    int                 load_bl_res;
    // int                 pin_bl_res;
    struct block*       requested_bl = NULL;
    struct cell_common* requested_cl = NULL;

    load_bl_res = storage_load_block(bl_info, searched_cld.bl_d, &requested_bl);
    if (load_bl_res == 1) { // block should be already loaded
        return 5;
    }
    else if (load_bl_res != 0) {
        return 3;
    }
    // Calculate cell address using pointer arithmetic
    requested_cl = (struct cell_common*) blocks_get_block_contents_start(requested_bl) + searched_cld.cl_d;
    *found_cell_wrap = malloc(sizeof(struct cell));
    if (*found_cell_wrap == NULL) {
        return 2;
    }
    /// No need in concurency
    // pin_bl_res = storage_pin_block(bl_info, searched_cld, pin_mode);
    // if (pin_bl_res == 1) {
    //     // wait for unpin or whatever
    //     free(*found_cell);
    //     return 1;
    // }
    // else if (pin_bl_res != 0) {
    //     free(*found_cell);
    //     return 4;
    // }
    **found_cell_wrap = (struct cell) {
        .cl_desc = searched_cld,
        .type = blocks_get_block_data_type(requested_bl),
        .ptr.common = requested_cl
    };
    return 0;
}

/**
 * @brief Remove one pin from the cell's block and release the cell.
 *
 * Use with cells from blocks
 * 
 * @param bl_info 
 * @param cell 
 * @return int 0 - Success
 * @return int 1 - Failed to unpin block
 */
int cells_release_cell_ptr(struct blocks_info* const bl_info, const enum cl_pin_mode pin_mode, struct cell* const cell) {
    if (storage_unpin_block(bl_info, cell -> cl_desc, pin_mode) != 0) {
        return 1;
    }
    free(cell);
    return 0;
}
