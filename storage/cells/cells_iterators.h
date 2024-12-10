#ifndef CELLS_ITERATORS_H
#define CELLS_ITERATORS_H

#include "blocks/block_info.h"
#include "utils/iterators/iterators.h"


typedef struct {
    iterator base;
    struct blocks_info* stor_bl_info;
    struct cell* root_cell;
} root_cell_iterator;

bool root_cell_move_next(iterator* it);

iterator* new_root_cell_iterator(struct blocks_info* const bl_info, struct cell* const root_cell);


typedef struct array_iterator {
    iterator base;
    size_t next_index;
    size_t arr_elem_size;
    size_t arr_lenght;
    void* arr_start;

} array_iterator;

iterator* new_array_iterator(void* arr_start, const size_t arr_len, const size_t arr_elem_size, fp_destroy_item arr_dstr);

iterator* new_cell_type_array_iter(void);

bool array_iter_move_next(iterator* it);



//============================no need

// iterator* new_dfs_cell_iterator(...);


// mb dfs-ный итератор должен хранить инф-ю о посещённых блоках
// и что-то ещё для продолжения поиска с места остановки
typedef struct { // мб только делает dfs, отдельный итер для 
    iterator self;
    

} dfs_cell_iterator;


/// Итератор по мета-инфе должен содержать что-то релевантное для себя
//



bool dfs_move_next(iterator* self);

// bool meta_inf_move_next(iterator* self);

#endif
