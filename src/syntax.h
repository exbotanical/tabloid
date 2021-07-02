#ifndef SYNTAX_H
#define SYNTAX_H

#include "common.h"

void highlight_syntax(t_row *row);

int map_syntax_to_color(int hl);

void resolve_syntax(void);

#endif
