#ifndef BLOCK_CLI_H
#define BLOCK_CLI_H

#include "blocks/block_types_pub.h"

#include <stdio.h>
#include <stdbool.h>

int log_block_info(FILE* log_stream, struct block* block, bool show_cell_layout);

#endif
