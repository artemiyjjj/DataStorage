#ifndef ITERATORS_H
#define ITERATORS_H

#include <stdbool.h>

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
