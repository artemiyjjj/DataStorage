#ifndef CELLS_ITERATORS_H
#define CELLS_ITERATORS_H

#include "blocks/block_info.h"
#include "glib.h"
#include "utils/iterators/iterators.h"


typedef struct {
    iterator base;
    struct blocks_info* stor_bl_info;
    struct cell* root_cell;
} root_cell_iterator;

bool root_cell_move_next(iterator* it);

iterator* new_root_cell_iterator(struct blocks_info* const bl_info, struct cell* const root_cell);


// Node here is cell_obj

iterator* new_single_item_iterator(struct cell* node);

// base.current points to struct cell
typedef struct children_iterator {
    iterator base;
    struct blocks_info* bl_info;
    struct cl_desc next_cell_desc;
} children_iterator;

iterator* new_node_imm_children_iterator(struct blocks_info* const bl_info, struct cell* node);

iterator* new_node_attr_iterator(struct blocks_info* const bl_info, struct cell* node);

bool imm_children_iter_move_next(iterator* it);

bool attr_iter_move_next(iterator* it);


typedef struct all_children_iterator {
    iterator base;
    struct blocks_info* bl_info;
    GQueue* iterator_stack;
} all_children_iterator;

iterator* new_node_all_children_iterator(struct blocks_info* const bl_info, struct cell* node);

bool all_children_iter_move_next(iterator* it);


typedef struct attr_filter_name_iterator {
    filter_iterator base;
    struct blocks_info* bl_info;
} attr_filter_name_iterator;

// Needs to select name cells, which can not be done in testFunc
iterator* new_attr_name_filter_iterator(iterator* base, struct blocks_info* const bl_info, char* expected_name);

bool attr_name_iter_move_next(iterator* it);


#endif
