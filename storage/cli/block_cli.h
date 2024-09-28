#ifndef BLOCK_CLI_H
#define BLOCK_CLI_H

#include "blocks/block_types.h"
#include "blocks/block_creat.h"

#include <stdio.h>

int fprintf_block_info(FILE* log_stream, struct block* block);

#endif
