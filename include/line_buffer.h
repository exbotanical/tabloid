#ifndef LINE_BUFFER_H
#define LINE_BUFFER_H

#include "libutil/libutil.h"
#include "piece_table.h"

typedef struct {
  array_t  *line_starts;
  buffer_t *line_buffer;
} render_state_t;

render_state_t *render_state_init(void);
void            render_state_free(render_state_t *self);
void            render_state_refresh(render_state_t *self, piece_table_t *pt);

#endif /* LINE_BUFFER_H */
