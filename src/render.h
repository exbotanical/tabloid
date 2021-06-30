#ifndef RENDER_H
#define RENDER_H

struct extensible_buffer;

void draw_msg_bar(struct extensible_buffer* e_buffer);

void draw_rows(struct extensible_buffer* e_buffer);

void draw_stats_bar(struct extensible_buffer* e_buffer);

void set_stats_msg(const char* fmt, ...);

#endif
