
#include <stdlib.h>

#include "exception.h"

static inline void*
xmalloc (size_t sz) {
  void* ptr;
  if ((ptr = malloc(sz)) == NULL) {
    panic("[xmalloc::%s] failed to allocate memory\n", __func__);
  }

  return ptr;
}
