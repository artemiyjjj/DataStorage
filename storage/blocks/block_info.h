#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include "block_info_pub.h"
#include "blocks/block_types_pub.h"

#include <glib.h>


/**
 * @brief 
 * 
 * @field block_size - size of one structural unit of storage, also size of 
 * page mapping for storage file layer
 * @field loaded_blocks_table - hash table with mapping of block descriptors
 * and all blocks loaded into memory - i.e. working set of blocks
 * @field loaded_meta_blocks_table - hash table with mapping of bl_desc of meta
 * blocks and bl_desc of loaded corresponding data blocks
 * @field cell_insertion_candidates_table - hash table with mapping of cl_type
 * and dynamic queue of bl_desc of data blocks to keep track of places where
 * new cell can be inserted to avoid parsing block's headers or meta cells in 
 * header blocks on each cell insertion
 */
struct blocks_info {
    int storage_fd;
    size_t block_size; // move to function (posix_get_page...)
    // hashset with bd's
    // which could store time of blocks being loaded in memory for auto remove them
    // (for balanced mem consuption)

    // <bl_desc*, struct block*>
    GHashTable* header_blocks_table;

    // <bl_desc*, struct block*>
    GHashTable* loaded_blocks_table;
    // GHashTable* loaded_meta_blocks_table;

    size_t insert_candidates_queue_length;
    /// <enum cl_type*, PriorityQueue<bl_desc>*>
    GHashTable* cell_insertion_candidates_table;
    // - очередь отсортирована по возрастанию, чтобы использовать пространство максимально эффективно
    // нужно как то за константу искать подходящую длинну
    // Идея - половина имеющихся блоков - где мало места, вторая половина - больше всего
    // Минимум должен быть 1 блок

    GQueue* free_blocks; // очередь используется для выбора места нового блока при создании
    // Создается при инициализаци программы при сканировании cell_block в каждом block_header (в перспективе отключаемо)
};

// Operations with blocks meta-info

int storage_get_meta_info_of_block(struct blocks_info* const, const bl_desc rbd, struct cell** const cl_meta);

int storage_set_meta_info_of_block(struct blocks_info* const, const struct block* const modifying_bl, struct cell* cl_meta);

int storage_update_insert_queries(struct blocks_info* const, const bl_desc hint);

int storage_update_block_meta_info(struct blocks_info* const, struct block* const modified_block, const struct block_state new_bl_state);


// Operations with blocks

bl_desc storage_get_header_desc_of(const size_t bl_size, const bl_desc member_bd);

size_t storage_get_bl_position_in_header_group(const size_t bl_size, const bl_desc bl_desc);

size_t storage_get_header_group_size(const size_t bl_size);

bool storage_try_fit_cell_to_block(struct blocks_info* const, const bl_desc bld, const struct cell* const new_cl);


// Operations with blocks_info state

int storage_get_storage_fd(const struct blocks_info* const);

size_t storage_get_block_size(const struct blocks_info* const);

bl_desc storage_get_last_bl_desc(const struct blocks_info* const);

void storage_set_last_bl_desc(const struct blocks_info* const, const bl_desc new_last_bl_desc);

struct cl_desc storage_get_root_cell(const struct blocks_info* const);

void storage_set_root_cell(const struct blocks_info* const, const struct cl_desc root_cl_desc);


bl_desc storage_use_free_bl_desc(const struct blocks_info* const);

void storage_append_free_bl_desc(struct blocks_info* const, const bl_desc free_bl_desc);

bl_desc storage_append_desc_of_loaded_bl(struct blocks_info* const, const struct block* const new_bl);

bl_desc storage_remove_desc_of_loaded_bl(struct blocks_info* const, const struct block* const new_bl);


size_t storage_get_insert_cand_size(const struct blocks_info* const);

#endif
