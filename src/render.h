#ifndef RENDER_H
#define RENDER_H

#include "../deps/libutil/buffer.h"

void draw_msg_bar(Buffer* e_buffer);

void draw_rows(Buffer* e_buffer);

void draw_stats_bar(Buffer* e_buffer);

void set_stats_msg(const char* fmt, ...);

#endif
