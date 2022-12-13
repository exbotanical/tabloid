#ifndef SYNTAX_H
#define SYNTAX_H

#include "common.h"

void syntax_highlight(Row *row);

int syntax_map_to_color(int hl);

void syntax_resolve(void);

#endif
