#ifndef RENDER_H
#define RENDER_H

struct extensible_buf;

void draw_msg_bar(struct extensible_buf *e_buffer);

void draw_rows(struct extensible_buf *e_buffer);

void draw_stats_bar(struct extensible_buf *e_buffer);

void set_stats_msg(const char *fmt, ...);

#endif
