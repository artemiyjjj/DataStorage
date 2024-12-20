#include "iterators.h"

#include "utils/mem.h"

#include <stdlib.h>

void* iterator_get_current(iterator* self) {
    return self -> current;
}

iterator* new_array_iterator(void** arr_start, const size_t arr_len, fp_destroy_item arr_dstr) {
    array_iterator* arr_it = myAllocStruct(array_iterator);
    if (!arr_it) {
        return NULL;
    }
    arr_it -> arr_start = arr_start;
    arr_it -> arr_lenght = arr_len;
    arr_it -> next_index = 0;
    arr_it -> base.current = NULL;
    arr_it -> base.move_next = array_iter_move_next;
    arr_it -> base.dstr_item = arr_dstr;
    return (iterator*) arr_it;
}

bool array_iter_move_next(iterator *it) {
    array_iterator* self = (array_iterator*) it;
    if (self -> next_index < self -> arr_lenght) {
        self -> base.current = self -> arr_start[self -> next_index];
        self -> next_index++;
        return true;
    } else {
        if (self -> base.dstr_item != NULL) {
            self -> base.dstr_item(self -> arr_start);
        }
        free(it);
        return false;
    }
}


iterator* new_filter_iterator(iterator* from, fp_test_condition test, void* param) {
    filter_iterator* it = malloc(sizeof(filter_iterator));
    it -> from = from;
    it -> test_condition = test;
    it -> param = param;
    it -> self.current = 0;
    it -> self.move_next = filter_iterator_step_forward;
    return (iterator*) it;
}

bool filter_iterator_step_forward(iterator *filter_iter) {
    filter_iterator* it = (filter_iterator*) filter_iter;
    iterator* from_it = it -> from;
    while (from_it -> move_next(from_it)) {
        void* item = from_it -> current;
        if (it -> test_condition(item, it -> param)) {
            it -> self.current = item;
            return true;
        }
    }
    free(it); // may remove to free explicitly
    return false;
}

void for_each(iterator* it, fp_consume action) {
    while (it -> move_next(it)) {
        action(it -> current);
    }
}
