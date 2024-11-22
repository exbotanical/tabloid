#include "status_bar.h"

#include <stdarg.h>

#include "globals.h"

void
status_bar_set_left_component_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.s_bar.left_component, sizeof(editor.s_bar.left_component), fmt, ap);
  va_end(ap);
}

void
status_bar_set_right_component_msg (const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(editor.s_bar.right_component, sizeof(editor.s_bar.right_component), fmt, ap);
  va_end(ap);
}
