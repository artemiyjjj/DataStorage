#ifndef QUERY_H
#define QUERY_H

#include "query_iterator.h"

#include <glib.h>
#include <stdbool.h>


typedef enum q_value_type {
    Q_VALUE_BOOLEAN,
    Q_VALUE_INTEGER,
    Q_VALUE_FLOAT,
    Q_VALUE_TEXT,
    Q_VALUE_NODE,
} q_value_type;

typedef union {
        bool boolean;
        int integer;
        float floating;
        char* text;
        struct cell* node;
} q_value_value;

typedef struct q_value {
    q_value_type type;
    union {
        bool boolean;
        int integer;
        float floating;
        char* text;
        struct cell* node;
    };
} q_value;

q_value newQueryValue(const q_value_type, const q_value_value);

q_value toBoolean(q_value value);

q_value toInteger(q_value value);

q_value toFloat(q_value value);

q_value toString(q_value value);

/**
 * @brief Conditions for a searched result set.
 *
 * In xpath terminology, conditions are name of Node and predicates
 */
typedef enum q_condition_op_type {
    // Q_COND_OP_NODE_NAME_MATCH,  // Node name matches provided reg exp
    // Q_COND_OP_CHILD_INDEX,     // check if child is n-th in list of children of node
    Q_COND_OP_TYPE_EQUALS,      /// Compare values of any type
    Q_COND_OP_TYPE_N_EQUALS,
    Q_COND_OP_TYPE_GREATER,
    Q_COND_OP_TYPE_GR_OR_EQ,
    Q_COND_OP_TYPE_LESS,
    Q_COND_OP_TYPE_LS_OR_EQ,    /// Logical operations
    Q_COND_OP_RELAT_TYPE_AND,
    Q_COND_OP_RELAT_TYPE_OR,
    Q_COND_OP_RELAT_TYPE_XOR,
    /// Node wrappers for literals
    Q_COND_OP_NODE_NAME,    // : str
    Q_COND_OP_NODE_VALUE,   // : str | int | bool
    Q_COND_OP_ATTR,    // : node - get attr by name
    Q_COND_OP_ATTR_VALUE,   // : str | int | bool | float
    /// Literals
    Q_COND_OP_TEXT,
    Q_COND_OP_INTEGER,
    Q_COND_OP_FLOAT,
} q_condition_op_type;

typedef struct q_condition_op {
    enum q_condition_op_type type;
    int opndsCount;
    union {
        struct q_condition_op **opnds;
        int integer;
        float floating;
        char* text;
    };
} q_condition_op;

q_value evaluateConditionGraph(struct blocks_info* const bl_info, struct cell* node, struct q_condition_op* op);

/**
 * @brief Defines set of elements which will be tested by conditions
 * 
 */
enum q_tree_operation {
    T_CONDITION = 0, // search for Node by it's name
    T_IMM_CHILDREN, //immediate (use as first operation in queue if start with root)
    T_ALL_CHILDREN, // bfs
    T_PARENT,
    T_SELF,
    T_ATTRIBUTES, // should be last level of queue or queue is invalid
};

typedef struct q_condition_level {
    int id;
    enum q_tree_operation lvl_tree_operation;
    //struct q_condition* list_conditions;
    struct q_condition_op* condition;
    struct q_condition_level* next;
} q_condition_level;

q_condition_level* query_new_cond_level(const enum q_tree_operation, q_condition_op* const cond_tree, q_condition_level* const prev_cond_level);

/**
 * @brief Structure for encounting updates of selected Nodes
 * or structure - attributes and children - of inserting nodes 
 */
// typedef struct node_diff {
//     q_param* node_name;
//     // List of q_param pairs
//     GList* attr_list;// g_list_alloc()
//     // List of node_diffs
//     GList* children_list;
//     struct node_diff* next_node;
// } node_diff;

enum query_type {
    QUERY_SELECT = 1,
    QUERY_INSERT,
    QUERY_UPDATE,
    QUERY_DELETE
};

typedef struct query {
    char q_magic[3];
    enum query_type q_type;
    // struct node_diff* node_changes;
    q_condition_level* list_cond_lvl;
} query;

query* query_new_query(const enum query_type, q_condition_level* const cond_lvl_list);

// Todo: make parser
query* parse_query(const char* query_string, const size_t query_string_lenght);

#endif
