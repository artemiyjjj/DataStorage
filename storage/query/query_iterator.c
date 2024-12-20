#include "query_iterator.h"

#include "cells/cell_types_pub.h"
#include "cells/cells_iterators.h"
#include "cells/cells_pub.h"
#include "cells/cell_types.h"
#include "query.h"
#include "utils/mem.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

static queryExecutionStackNode* stackPush(queryExecutionStackNode* node, struct q_condition_level* queryStep, struct iterator* it) {
    queryExecutionStackNode* newNode = myAllocStruct(queryExecutionStackNode);
    newNode->it = it;
    newNode->queryStep = queryStep;
    newNode->next = node;
    return newNode;
}

static queryExecutionStackNode* stackPop(queryExecutionStackNode* node) {
    struct queryExecutionStackNode* next = node->next;
    free(node);  // iterators should be destroyed automatically
    return next;
}


struct iterator* apply_query(struct blocks_info* const bl_info, struct cell* const root, struct q_condition_level* query) {
    struct iterator* it = new_single_item_iterator(root);
    queryIterator* qIt = myAllocStruct(queryIterator);
    qIt -> stack = stackPush(NULL, query, it);
    qIt -> bl_info = bl_info;
    qIt -> me.move_next = queryMoveNext;
    qIt -> me.current = NULL;
    return (iterator*) qIt;
}

void* queryGetCurrent(queryIterator* qIt) {
    return qIt->me.current;
}


/*
 *  /a/b/c/(
        (/x/y[a=3]/*) | (/z[]//*)
    )

    /a/b[x=4|y=5]

 *  T_IMM_CHILDREN, T_CONDITION[type == x], T_IMM_CHILDREN, T_CONDITION[type == y], T_CONDITION[a == 3], T_IMM_CHILDREN
 */


bool queryMoveNext(iterator* queue_iter) {
    queryIterator* qIt = (queryIterator*) queue_iter;
    while (qIt->stack != NULL) {
        while (qIt->stack->it->move_next(qIt->stack->it)) {
            struct cell* node = qIt -> stack -> it -> current;
            struct q_condition_level* treeOp = qIt->stack->queryStep;
            if (treeOp) {
                // printf("q_condition_level #%d\n", treeOp->id);
                switch (treeOp->lvl_tree_operation) {
                    case T_CONDITION: {
                        if (toBoolean(evaluateConditionGraph(qIt -> bl_info, node, treeOp -> condition)).boolean) {
                            struct iterator* it = new_single_item_iterator(node);
                            qIt->stack = stackPush(qIt -> stack, treeOp->next, it);
                        }
                    } break;
                    case T_IMM_CHILDREN: {
                        iterator* childrenIterator = new_node_imm_children_iterator(qIt -> bl_info, node);
                        qIt->stack = stackPush(qIt->stack, treeOp->next, childrenIterator);
                    } break;
                    case T_ALL_CHILDREN: {
                        iterator* childrenIterator = new_node_all_children_iterator(qIt -> bl_info, node);
                        qIt->stack = stackPush(qIt->stack, treeOp->next, childrenIterator);
                    } break;
                    case T_PARENT: {
                        struct cell* parent_node = cells_get_parent(qIt -> bl_info, node);
                        iterator* parentIterator = parent_node != NULL 
                            ? new_single_item_iterator(parent_node)
                            : new_array_iterator(NULL, 0, NULL);
                        qIt->stack = stackPush(qIt->stack, treeOp->next, parentIterator);
                    } break;
                    //case T_COMBINE: {
                    //    iterator* left = newSingleItemIterator(node);
                    //    qIt->stack = stackPush(qIt->stack, treeOp->nextLeft, left);
                    //    
                    //    iterator* right = newSingleItemIterator(node);
                    //    qIt->stack = stackPush(qIt->stack, treeOp->nextRight, right);
                    //}
                    default: {
                        // TODO log error
                        return false;
                    }
                }
            }
            else {
                printf("node %lu:%d\n", node->cl_desc.bl_d, node->cl_desc.cl_d);
                qIt->me.current = node;
                return true;
            }
        }
        qIt->stack = stackPop(qIt->stack);
    }
    free(qIt);
    return false;
}
