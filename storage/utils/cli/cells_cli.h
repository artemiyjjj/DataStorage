#ifndef CELLS_CLI_H
#define CELLS_CLI_H

#include "cells/cell_types.h"

#include <stdio.h>

void log_cell(FILE* log_stream, const struct cell* const cell);

void log_node(FILE* log_stream, const struct cell* const node);

#endif
