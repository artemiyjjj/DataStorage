#include "query.h"
#include "cells/cell_types_pub.h"
#include "cells/cells_iterators.h"
#include "cells/cells_pub.h"
#include "utils/iterators/iterators.h"
#include "utils/mem.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


q_value newQueryValue(const q_value_type q_val_type, const q_value_value new_value) {
    q_value q_val;
    q_val.type = q_val_type;
    q_val.text = new_value.text;
    return q_val;
}


q_value toBoolean(q_value value) {
    struct q_value new_value;
    new_value.type = Q_VALUE_BOOLEAN;
    switch (value.type) {
        case Q_VALUE_BOOLEAN: {
            return value;
        } break;
        case Q_VALUE_INTEGER: {
            new_value.boolean = value.integer != 0 ? true : false;
        } break;
        case Q_VALUE_FLOAT: {
            new_value.boolean = nearbyintf(value.floating) != 0.0;
        } break;
        case Q_VALUE_TEXT: { // string is not empty
            new_value.boolean = strcmp("\0", value.text) ? true : false;
        } break;
        case Q_VALUE_NODE: {
            new_value.boolean = value.node ? true : false;
        } break;
        default: break;
    }
    return new_value;
}

q_value toInteger(q_value value) {
    struct q_value new_value;
    new_value.type = Q_VALUE_INTEGER;
    switch (value.type) {
        case Q_VALUE_BOOLEAN: {
            new_value.integer = value.boolean ? 1 : 0;
        }; break;
        case Q_VALUE_INTEGER: {
            return value;
        }; break;
        case Q_VALUE_FLOAT: {
            new_value.integer = round(value.floating);
        }; break;
        case Q_VALUE_TEXT: {
            new_value.integer = atoi(value.text);
        }; break;
        case Q_VALUE_NODE: {
            new_value.integer = value.node ? 1 : 0;
        }; break;
        default: break;
    }
    return new_value;
}

q_value toString(q_value value) {
    struct q_value new_value;
    new_value.type = Q_VALUE_TEXT;
    char val[10];
    switch (value.type) {
        case Q_VALUE_BOOLEAN: {
            strcpy(val, value.boolean ? "true\0" : "false\0");
        }; break;
        case Q_VALUE_INTEGER: {
            sprintf(val, "%d", value.integer);
        }; break;
        case Q_VALUE_FLOAT: {
            sprintf(val, "%.3f", value.floating);
        }; break;
        case Q_VALUE_TEXT: {
            return value;
        }; break;
        case Q_VALUE_NODE:
        default: break;
    }
    char* mem = malloc(strlen(val));
    strcpy(mem, val);
    new_value.text = mem;
    return new_value;
}

static void coerceOperands(q_condition_op_type opKind, q_value* values) {
    switch (opKind) {
        // case Q_COND_OP_CHILD_INDEX: {
        //     values[0] = toInteger(values[0]);
        // }; break;

        /// Comparasing operations migth take parameters of any type
        case Q_COND_OP_TYPE_EQUALS:
        case Q_COND_OP_TYPE_N_EQUALS:
        case Q_COND_OP_TYPE_GREATER:
        case Q_COND_OP_TYPE_GR_OR_EQ:
        case Q_COND_OP_TYPE_LESS:
        case Q_COND_OP_TYPE_LS_OR_EQ: 
            break;
        case Q_COND_OP_RELAT_TYPE_AND:     
        case Q_COND_OP_RELAT_TYPE_OR:        
        case Q_COND_OP_RELAT_TYPE_XOR: {
            values[0] = toBoolean(values[0]);
            values[1] = toBoolean(values[1]);
        }; break;
        case Q_COND_OP_TEXT: 
        case Q_COND_OP_INTEGER:
        case Q_COND_OP_FLOAT:
        default:
            // do nothing
            break;
    }
}

static int cmp_values(q_value* values) {
    switch (values[0].type) {
        case Q_VALUE_BOOLEAN: {
            return values[0].boolean == values[1].boolean ? 0 : 1;
        } break;
        case Q_VALUE_INTEGER: {
            return values[0].integer - values[1].integer;
        } break;
        case Q_VALUE_FLOAT: {
            return values[0].floating - values[1].floating;
        } break;
        case Q_VALUE_TEXT:    return strcmp(values[0].text, values[1].text);
        case Q_VALUE_NODE:    return cells_cmp(values[0].node, values[1].node);
        default:              return 1;
    }
}

q_value evaluateConditionGraph(struct blocks_info* const bl_info, struct cell* node, struct q_condition_op* op) {
    bool isLiteral = op->type == Q_COND_OP_TEXT || op->type == Q_COND_OP_INTEGER;
    // Literals shouldn't have child condisions
    q_value* values = isLiteral ? NULL : myAllocArray(q_value, op->opndsCount);
    for (int i = 0; i < op->opndsCount; i++) {
        values[i] = evaluateConditionGraph(bl_info, node, op->opnds[i]);
    }

    q_value result;
    result.type = Q_VALUE_BOOLEAN;

    coerceOperands(op->type, values);

    switch (op->type) {
        // case Q_COND_OP_CHILD_INDEX: {
        //     int child_counter = 0;
        //     struct cell* child = cells_get_first_child(bl_info, node);
        //     while (child && child_counter != values[0].integer) {
        //     }
        // }; break;
        case Q_COND_OP_TYPE_EQUALS: {
            result.boolean = cmp_values(values) == 0;
        }; break;
        case Q_COND_OP_TYPE_N_EQUALS: {
            result.boolean = cmp_values(values) != 0;
        } break;
        case Q_COND_OP_TYPE_GREATER: {
            result.boolean = cmp_values(values) > 0;
        } break;
        case Q_COND_OP_TYPE_GR_OR_EQ: {
            result.boolean = cmp_values(values) >= 0;
        } break;
        case Q_COND_OP_TYPE_LESS: {
            result.boolean = cmp_values(values) < 0;
        } break;
        case Q_COND_OP_TYPE_LS_OR_EQ: {
            result.boolean = cmp_values(values) <= 0;
        } break;
        case Q_COND_OP_RELAT_TYPE_AND: {
            result.boolean = values[0].boolean && values[1].boolean;
        }; break;
        case Q_COND_OP_RELAT_TYPE_OR: {
            result.boolean = values[0].boolean || values[1].boolean;
        }; break;
        case Q_COND_OP_RELAT_TYPE_XOR: {
            result.boolean = values[0].boolean ^ values[1].boolean;
        }; break;
        case Q_COND_OP_NODE_NAME: {
            result.type = Q_VALUE_TEXT;
            struct cell* name_cell = cells_node_get_name_cell(bl_info, node);
            if (name_cell == NULL) {
                result.text = myAllocStruct(char);
                result.text[0] = '\0'; // debug
            }
            // Leak
            result.text = cells_get_simple_type_value(name_cell).string;
        }; break;
        case Q_COND_OP_NODE_VALUE: {
            struct cell* value_cell = NULL;

            value_cell = cells_node_get_value_cell(bl_info, node);
            if (value_cell == NULL) { // node migth not have value
                result.boolean = true;
                break;
            }
            cells_simple_values value = cells_get_simple_type_value(value_cell);
            switch (cells_get_cell_type(value_cell)) {
                case CELL_INT32: {
                    result.type = Q_VALUE_INTEGER;
                    result.integer = value.int32;
                }; break;
                case CELL_FLOAT32: {
                    result.type = Q_VALUE_FLOAT;
                    result.floating = value.float32;
                }; break;
                case CELL_BOOL: {
                    result.type = Q_VALUE_BOOLEAN;
                    result.boolean = value.boolean;
                }; break;
                case CELL_STRING: {
                    result.type = Q_VALUE_TEXT;
                    result.text = value.string;
                }; break;
                default: break;
            }
        }; break;
        case Q_COND_OP_ATTR: { // uses `Q_COND_OP_TEXT` as attr name
            result.type = Q_VALUE_NODE;
            result.node = NULL;
            iterator* attr_it = new_node_attr_iterator(bl_info, node);
            iterator* filter_by_name_it = new_attr_name_filter_iterator(attr_it, bl_info, values[0].text);
            while (filter_by_name_it -> move_next(filter_by_name_it)) {
                // Attrs have unique names, so there should be one occurence
                /// Copy found cell - iterator will free it's copy
                result.node = select_cell(bl_info, cells_get_cell_desc(filter_by_name_it  -> current));
            }
        }; break;
        case Q_COND_OP_ATTR_VALUE: { // uses `Q_COND_OP_ATTR`
            result.boolean = false;
            struct cell* value_cell = NULL;
            struct cell* found_attr_cell = values[0].node;
            if (found_attr_cell == NULL) {
                // result is false
                break;
            }
            value_cell = cells_attr_get_value_cell(bl_info, found_attr_cell);
            if (value_cell == NULL) { // attr migth be boolean
                result.boolean = true;
                break;
            }
            cells_simple_values value = cells_get_simple_type_value(value_cell);
            switch (cells_get_cell_type(value_cell)) {
                case CELL_INT32: {
                    result.type = Q_VALUE_INTEGER;
                    result.integer = value.int32;
                }; break;
                case CELL_FLOAT32: {
                    result.type = Q_VALUE_FLOAT;
                    result.floating = value.float32;
                }; break;
                case CELL_BOOL: {
                    result.type = Q_VALUE_BOOLEAN;
                    result.boolean = value.boolean;
                }; break;
                case CELL_STRING: {
                    result.type = Q_VALUE_TEXT;
                    result.text = value.string;
                }; break;
                default: break;
            }
        }; break;
        case Q_COND_OP_TEXT: {
            result = (q_value){ .type= Q_VALUE_TEXT, .text = op->text };
        }; break;
        case Q_COND_OP_INTEGER: {
            result = (q_value){ .type = Q_VALUE_INTEGER, .integer = op->integer };
        }; break;
        case Q_COND_OP_FLOAT: {
            result = (q_value) { .type = Q_VALUE_FLOAT, .floating = op -> floating };
        }; break;
        default:
            // TODO log error
            break;
    }

    if (values) {
        free(values);
    }
    
    return result;
}

// static int q_condition_level_counter = 0; 

q_condition_level* query_new_cond_level(const enum q_tree_operation tree_op_type, q_condition_op* const cond_tree, q_condition_level* const prev_cond_lvl) {
    q_condition_level* cond_lvl = myAllocStruct(q_condition_level);
    // cond_lvl -> id =  q_condition_level_counter++;
    cond_lvl -> lvl_tree_operation = tree_op_type;
    cond_lvl -> condition = cond_tree;
    cond_lvl -> next = NULL;
    if (prev_cond_lvl != NULL) {
        prev_cond_lvl -> next = cond_lvl;
    }
    return cond_lvl;
}
