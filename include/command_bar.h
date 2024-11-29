#ifndef COMMAND_BAR_H
#define COMMAND_BAR_H

#include "line_editor.h"

#define COMMAND_BAR_PREFIX        "> "
#define COMMAND_BAR_PREFIX_OFFSET 2

typedef enum { CB_INPUT, CB_MESSAGE } command_mode_t;

void command_bar_clear(line_editor_t* self);
void command_bar_process_command(line_editor_t* self);
void command_bar_set_message_mode(line_editor_t* self, const char* fmt, ...);

#endif /* COMMAND_BAR_H */
