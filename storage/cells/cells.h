#ifndef CELLS_H
#define CELLS_H

/** Functions for inner interactions of cell's layer. Access to cells
 * with cell descriptions should be hidden from other layers.
 */

#include "blocks/block_info_pub.h"
#include "blocks/block_types_pub.h"

int cells_get_cpy_cell(struct blocks_info* const, const struct cl_desc searched_cld, struct cell** const found_cell);

void cells_free_cpy_cell(struct cell** cell);

int cells_get_cell_ptr(struct blocks_info* const, const struct cl_desc searched_cld, const enum cl_pin_mode, struct cell** const found_cell);

int cells_release_cell_ptr(struct blocks_info* const, const enum cl_pin_mode, struct cell* const cell);


#endif
