#include "mode.h"

#include "command_bar.h"
#include "globals.h"

void
mode_chmod (editor_mode_t next) {
  if (editor.mode == COMMAND_MODE && next == EDIT_MODE) {
    command_bar_clear(&editor.c_bar);
  }

  editor.mode = next;
}
