#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include "block_types.h"

// Можно сделать структуру (сет), хранящую инфу о блоках (fd и адрес начала) и отдающую наружу дескриптор блока (порядковый номер блока в файле).
// Это упростит вызовы чтения, записи и удаления блока, но придется писать обертки для
struct block_list {
    struct block* block;
    struct block_list* next;
    struct block_list* prev;
};


struct blocks_info {
    int storage_fd;
    unsigned int block_size; // move to function
    // hashset with bd's
    // which could store time of blocks being loaded in memory for auto remove them
    // (for balanced mem consuption)
    struct block_list* header_blocks_list;
    //Change to hash set or smth to be able to get block at O(1)
    // Might also find closest block to save child near parent 
    struct block_list* loaded_blocks_list;
    // lists of static Data blocks by storing type and list of dyn data blocks
    // struct priority_queue* queue_fix_cell_size // - длинна клетки у блоков для разных типов
    // будет разной, поэтому придется делать и очередь под каждый тип
    //struct priority_queue* queue_var_cell_size // - очередь отсортирована по возрастанию,
    // нужно как то за константу искать подходящую длинну
    int (*get_last_bl_desc)(const struct blocks_info* const, bl_desc* const);
    int (*set_last_bl_desc)(const struct blocks_info* const, const bl_desc);
    int (*get_root_cell)(const struct blocks_info* const, bl_desc* const, bl_offset* const);
    int (*set_root_cell)(const struct blocks_info* const, const bl_desc, const bl_offset);
};

// Operations with block list

// Не забыть исправить реализацию, когда сделаю hashset
int add_block_to_list(struct blocks_info* const bl_info, const bl_desc bd, void* const block_addr, struct block** block);

int get_block_from_list(const struct blocks_info* const bl_info, const bl_desc bd, const bool is_header, struct block** block);

int remove_block_from_list(struct blocks_info* const bl_info, const bl_desc bd, bool is_header);


// Operations with blocks_info

int get_last_bl_desc(const struct blocks_info* const bl_info, bl_desc* const bl_desc);

int set_last_bl_desc(const struct blocks_info* const bl_info, const bl_desc new_last_bl_desc);

int get_root_cell(const struct blocks_info* const bl_info, bl_desc* const root_cl_desc, bl_offset* const root_cl_offset);

int set_root_cell(const struct blocks_info* const bl_info, const bl_desc root_cl_desc, const bl_offset root_cl_offset);

#endif
