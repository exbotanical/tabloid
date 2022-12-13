#ifndef RENDER_H
#define RENDER_H

#include "../deps/libutil/buffer.h"

void draw_msg_bar(Buffer* buffer);

void draw_rows(Buffer* buffer);

void draw_stats_bar(Buffer* buffer);

void set_stats_msg(const char* fmt, ...);

#endif
