#include "storage.h"

#include "blocks/block_info.h"
#include "blocks/block_info_pub.h"
#include "blocks/blocks.h"
#include "cells/cell_types.h"
#include "cells/cell_types_pub.h"
#include "cells/cells_iterators.h"
#include "cells/cells_pub.h"
#include "storage_file.h"
#include "utils/cli/block_cli.h"
#include "utils/iterators/iterators.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


/* Storage creates, initializes, closes storage file and also serves
as an interface between queries and blocks & cells layers.
*/

/**
 * @brief Create a open storage object
 * 
 * @param filename 
 * @param bl_info 
 * @return int 0 - Success
 * @return int 1 - Failed to create a new storage file
 * @return int 2 - 
 * @return int 3 - Failed to create header block
 * @return int 4 - Failed to create root cell
 * @return int 5 - Failed to create root cell iterator
 * @return int 6 - Failed to find a place for a root cell
 * @return int 7 - Failed to insert root cell
 * @return int 
 */
static int create_open_storage(const char* const filename, struct blocks_info* const bl_info) {
    struct block*   block;
    struct cell_obj root_cell;
    struct cell*    root_cell_wrap;
    struct cell*    found_cell_wrap;
    struct cl_desc  inserted_cell_desc;

    if (open_file(filename, true, &(bl_info -> storage_fd)) != 0) {
        return 1;
    }
    // Create header block for storage meta info
    if (blocks_create_block(bl_info, BLOCK_HEAD, CELL_BLOCK, &block) != 0) {
        close(bl_info -> storage_fd);
        return 3;
    }
    // Create root cell and insert it into the storage
    root_cell = (cell_obj) { // maybe taken to function to hide `cell_types` from this layer
        .type = CELL_OBJECT,
        .attrs = cells_get_default_cld(),
        .ancestor = cells_get_default_cld(),
        .children = cells_get_default_cld(),
        .next_sibling = cells_get_default_cld(),
        .prev_sibling = cells_get_default_cld(),
        .value = cells_get_default_cld()
    };
    root_cell_wrap = create_cell(CELL_OBJECT, cells_get_cell_type_size(CELL_OBJECT), &root_cell);
    if (root_cell_wrap == NULL) {
        return 4;
    }
    // create block data
    // insert cell there
    // get cl_desc of inserted root cell
    iterator* root_cl_iter = new_root_cell_iterator(bl_info, root_cell_wrap);
    if (root_cl_iter == NULL) {
        return 5;
    }
    if (root_cl_iter -> move_next(root_cl_iter)) {
        found_cell_wrap = (struct cell*) root_cl_iter -> current;
    } else {
        return 6;
    }
    inserted_cell_desc = insert_cell(bl_info, root_cell_wrap, found_cell_wrap);
    if (inserted_cell_desc.cl_d == UNDEF_CL_DESC) {
        return 7;
    }
    storage_set_root_cell(bl_info, inserted_cell_desc);
    return 0;
}

/**
 * @brief 
 * 
 * @param bl_info 
 * @return int 0 - Successfully loaded all header blocks
 * @return int 1 - Failed to load a block
 */
static int load_header_blocks(struct blocks_info* const bl_info) {
    struct block* header_block = NULL;
    size_t block_size = storage_get_block_size(bl_info);
    size_t blocks_in_group = storage_get_header_group_size(block_size);
    bl_desc last_bl_desc_in_storage = storage_get_last_bl_desc(bl_info);
    bl_desc cur_header_bd = 0;

    do {
        if (storage_load_block(bl_info, cur_header_bd, &header_block) != 0) {
            return 1;
        }
        // dummy debug
        log_block_info(stdout, header_block, false);
        if (header_block == NULL) {
            return 1;
        }
        assert(header_block -> type == BLOCK_HEAD);
        // Calculate next
        cur_header_bd += blocks_in_group;
    } while (cur_header_bd <= last_bl_desc_in_storage);
    return 0;
}

/**
 * @brief Open exsisting storage or create and initialize a new one.
 * During this step, all header blocks from storage are being loaded into memory.
 * @param filename str - name of storage file
 * @param bl_info - NULLABLE structure for manipulating information about the storage 
 * @return int 0 - success
 * @return int 1 - failed to allocate memory for `bl_info`
 * @return int 2 - failed to open or create storage
 * @return int 3 - failed to load header blocks
 */
int init_storage(const char* filename, struct blocks_info** bl_info) {
    if (*bl_info = storage_new_blocks_info(), *bl_info == NULL) {
        return 1;
    }

    if (open_file(filename, false, &((*bl_info) -> storage_fd)) == 0) {
        fprintf(stdout, "Storage \"%s\" is opened.\n", filename);
    } else if (create_open_storage(filename, *bl_info) == 0) {
        // fprintf(stderr, "Failed to open storage with filename \"%s\".\n", filename);
        // fprintf(stdout, "Trying to create a new storage with filename \"%s\".\n", filename);
        fprintf(stdout, "Storage \"%s\" is created.\n", filename);
    } else {
        fprintf(stderr, "Failed to open or create storage.\n");
        return 2;
    }

    
    if (load_header_blocks(*bl_info) != 0) {
        close_storage(bl_info);
        return 3;
    }
    // log storage info (amount of blocks, elements, etc)

    /// Parse cell_blocks info, fill bl_info structures
    // init `cell_insertion_candidates_table` with first N candidates (parse cl blocks stay the ones with less free space and closer to storage start)
    // 
    return 0;
}

void close_storage(struct blocks_info** bl_info) {
    storage_destroy_blocks_info(bl_info);
}

/* 
Executes select query to find elements (cl_desc) for insertion
of element (or miltiple elements)
*/
// int insert(struct query* query, struct element* elem) {
//     insert_cells()
//      
//     return 1;
// }

// int insert_cells(struct blocks_info* const bl_info, struct cell* const cell_to_insert, iterator* const found_cell_iterator) {
//     while (found_cell_iterator -> move_next(found_cell_iterator)) {
//         find_new_cell_place();
//         if (bl_info -> cell_insertion_candidates_table...) { // if no candidates left, update candidates queue
//             update_cl_ins_candidates(bl_info);
//         }
//     }
// }


// /**
//  * @brief 
//  *
//  * 
//  * @param bl_info 
//  * @param cell_to_insert 
//  * @param found_cell_iterator - cell iterator
//  * @return int 
//  */
// int insert_cells(struct blocks_info* const bl_info, struct cell *const cell_to_insert, iterator *const found_cell_iterator) {
//     int add_child_res;
    
//     assert(cell_to_insert != NULL || found_cell_iterator != NULL);

//     while (found_cell_iterator -> move_next(found_cell_iterator)) {
//         add_child_res = cell_obj_add_child(bl_info, found_cell_iterator -> current, cell_to_insert -> cl_desc);
//         if (add_child_res != 0) {
//             return 1;
//         }
//     }
//     return 0;
// }


// static bool testCondition(void* node, struct q_condition* cond) {
//     // TODO
// } 

// typedef struct queryExecutionStackNode {
//     struct queryExecutionStackNode* next;
//     struct q_condition_level* queryStep;
//     struct iterator* it;
// } queryExecutionStackNode;

// static queryExecutionStackNode* stackPush(queryExecutionStackNode* node, struct q_condition_level* queryStep, struct iterator* it) {
//     queryExecutionStackNode* newNode = myAllocStruct(queryExecutionStackNode);
//     newNode->it = it;
//     newNode->queryStep = queryStep;
//     newNode->next = node;
//     return newNode;
// }

// static queryExecutionStackNode* stackPop(queryExecutionStackNode* node) {
//     struct queryExecutionStackNode* next = node->next;
//     free(node);
//     return next;
// }

// typedef void(*arrayItDtor)(void** arr, void* ctx);
// struct iterator* iteratorArrayNew(void** arr, int count, arrayItDtor dtor, void* dtorCtx);

// static struct iterator* newSingleItemItemIterator(void* item) {
//     void** items = myAllocStruct(void*);
//     items[0] = item;
//     return iteratorArrayNew(items,  1, dtor, NULL);
// }

// struct iterator applyQuery(struct cell root, struct q_condition_level* query) {
//     struct cell rootArr[1] = { root };
//     queryExecutionStackNode* stack = stackPush(NULL, query, rootIt);
    
//     while (stack != NULL) {
//         if (stack->it->move_next(stack->it)) {
//             void* node = stack->it->current(stack->it);
//             struct q_condition_level* treeOp = stack->queryStep;
//             switch (treeOp->lvl_tree_operation) {
//                 case T_CONDITION: {
//                     if (testCondition(node, treeOp->list_conditions)) {
//                         struct iterator* it = newSingleItemItemIterator(node);
//                         stack = stackPush(stack, treeOp->next, it);
//                     }
//                 } break;
//                 case T_IMM_CHILDREN:
//                 case T_ALL_CHIlDREN:
//                 case T_PARENT:
//                 default: // achtung
//             }
//         } else {
//             stack = stackPop(stack);
//         }
//     }

// }

