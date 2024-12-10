#ifndef STORAGE_H
#define STORAGE_H

#include "blocks/block_info_pub.h"

enum q_condition_param_type {
    Q_PARAM_STRING = 1,
    Q_PARAM_INT,
    Q_PARAM_BOOL,
    Q_PARAM_FLOAT
};

enum q_condition_type {
    Q_COND_TYPE_EQUALS = 1,
    Q_COND_TYPE_GREATER,
    Q_COND_TYPE_GR_OR_EQ,
    Q_COND_TYPE_LESS,
    Q_COND_TYPE_LS_OR_EQ,
    // for insert

    // for function-like conditions like 'ancestor' ...
};

enum q_condition_subject {
    Q_COND_ATTR = 1,
    Q_COND_OBJECT,
    Q_
};

union param_value {
    char* string_value;
    int int_value;
    bool bool_value;
    float float_value;
};

//test
struct q_condition {
    enum q_condition_param_type params_type;
    union param_value first_param_value;
    union param_value second_param_value;
    struct q_condition* next;
};

enum q_tree_operation {
    T_CONDITION = 0,
    T_IMM_CHILDREN, //immediate
    T_ALL_CHIlDREN, // bfs
    T_PARENT,
};

struct q_condition_level {
    struct q_operation* lvl_tree_operation;
    struct q_condition* list_conditions;
    struct q_condition_level* next;
};

enum query_type {
    QUERY_SELECT = 1,
    QUERY_INSERT,
    QUERY_UPDATE,
    QUERY_DELETE
};

struct query {
    char q_magic[3];
    enum query_type q_type;
    struct q_condition_level* list_cond_lvl;
};

int init_storage(const char* filename, struct blocks_info** bl_info);

void close_storage(struct blocks_info** bl_info);


int storage_select(struct blocks_info* const, struct query* query);

int storage_insert(struct blocks_info* const, struct query* query);

int storage_update(struct blocks_info* const, struct query* query);

int storage_delete(struct blocks_info* const, struct query* query);

int parse_query(const char* query_string, const size_t query_lenght, struct query** parsed_query);



// int insert(struct blocks_info* const bl_info, struct cell* const cell_to_insert, iterator* const found_cell_iterator);

// // int find(const struct blocks_info* const bl_info, const struct cl_desc cd, struct cell* const found_cell); // todo

// int select(struct condition* condition, );

// int update(...);

// // int update(struct block* const bl, const bl_offset bl_off, );

// int delete(...);


#endif // STORAGE_H
