#ifndef BLOCKS_H
#define BLOCKS_H

// Можно сделать структуру (сет), хранящую инфу о блоках (fd и адрес начала) и отдающую наружу дескриптор блока (порядковый номер блока в файле).
// Это упростит вызовы чтения, записи и удаления блока, но придется писать обертки для

typedef int bl_desc;

enum bl_type {
    BLOCK_HEAD = 0,

};

struct bl_manager {
    int fd;
    // hashset with bd's
    // which could store time of blocks being loaded in memory for auto remove them
    // (for balanced mem consuption)
};

int create_block(int fd, bl_desc bd, enum bl_type bt);

int sync_block(int fd, bl_desc db);

int delete_block(int fd, bl_desc bd);

#endif // BLOCKS_H
