#ifndef ITERATORS_H
#define ITERATORS_H

#include <stdbool.h>
#include <stdio.h>

struct iterator;

typedef bool(*fp_move_next)(struct iterator* it);

typedef void(*fp_destroy_item)(void* item);

typedef struct iterator {
    void* current;
    fp_move_next move_next;
    fp_destroy_item dstr_item;
} iterator;

void* iterator_get_current(iterator* self);

void iterator_empty_destroy(void* fake_item);


typedef struct ptr_array_iterator {
    iterator base;
    size_t next_index;
    size_t arr_lenght;
    void** arr_start;
} array_iterator;

iterator* new_array_iterator(void** arr_start, const size_t arr_len, fp_destroy_item arr_dstr);

bool array_iter_move_next(iterator* it);


typedef bool(*fp_test_condition)(void* item, void* param);

typedef struct {
    iterator self;
    iterator* from;
    void* param; // might change to condition or move conditions logic above (cell_filter_iterator, ...)
    fp_test_condition test_condition;
} filter_iterator;

iterator* new_filter_iterator(iterator* from, fp_test_condition test, void* param);

bool filter_iterator_step_forward(iterator* iterator);


typedef void(*fp_consume)(void* item);

void for_each(iterator* iterator, fp_consume action);

#endif
