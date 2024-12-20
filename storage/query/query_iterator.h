#ifndef QUERY_ITERATOR_H
#define QUERY_ITERATOR_H

#include "blocks/block_info_pub.h"
#include "utils/iterators/iterators.h"

typedef struct queryExecutionStackNode {
    struct queryExecutionStackNode* next;
    struct q_condition_level* queryStep;
    struct iterator* it;
} queryExecutionStackNode;

typedef struct queryIterator {
    struct iterator me;
    queryExecutionStackNode* stack;
    struct blocks_info* bl_info;
} queryIterator;

struct iterator* apply_query(struct blocks_info* const, struct cell* const root_node, struct q_condition_level* query);

bool queryMoveNext(iterator* qIt);

void* queryGetCurrent(queryIterator* qIt);

#endif
