#include "piece_table.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "calc.h"

seq_buffer_t*
seq_buffer_init (void) {
  seq_buffer_t* self = malloc(sizeof(seq_buffer_t));
  self->length       = 0;
  self->max_size     = 0;
  self->id           = 0;
  self->buffer       = buffer_init(NULL);

  return self;
}

void
seq_buffer_free (seq_buffer_t* self) {
  buffer_free(self->buffer);
  free(self);
}

event_stack_t*
event_stack_init (void) {
  event_stack_t* self  = malloc(sizeof(event_stack_t));
  self->event_captures = array_init();

  return self;
}

void
event_stack_free (event_stack_t* self) {
  array_free(self->event_captures, free);
  free(self);
}

bool
event_stack_empty (event_stack_t* self) {
  return array_size(self->event_captures) == 0;
}

piece_descriptor_range_t*
event_stack_last (event_stack_t* self) {
  if (array_size(self->event_captures) == 0) {
    return NULL;
  }
  // TODO: -1 indexing?
  return (piece_descriptor_range_t*)
    array_get(self->event_captures, array_size(self->event_captures) - 1);
}

void
event_stack_push (event_stack_t* self, piece_descriptor_range_t* pdr) {
  array_push(self->event_captures, (void*)pdr);
}

piece_descriptor_range_t*
event_stack_pop (event_stack_t* self) {
  piece_descriptor_range_t* pdr = array_pop(self->event_captures);
  assert(pdr);
  return pdr;
}

void
event_stack_clear (event_stack_t* self) {
  array_free(self->event_captures, free);
  self->event_captures = array_init();
}

piece_descriptor_range_t*
event_stack_back (event_stack_t* self, unsigned int index) {
  unsigned int size = array_size(self->event_captures);
  // TODO:
  assert(size > 0 && index < size);
  return (piece_descriptor_range_t*)array_get(self->event_captures, size - index - 1);
}

static int id_source = -2;  // TODO:

piece_descriptor_t*
piece_descriptor_init (void) {
  piece_descriptor_t* self = malloc(sizeof(piece_descriptor_t));
  self->id                 = id_source++;
  self->offset             = 0;
  self->length             = 0;
  self->buffer             = 0;
  self->next               = NULL;
  self->prev               = NULL;

  return self;
}

void
piece_descriptor_free (piece_descriptor_t* self) {
  free(self);
}

void
piece_descriptor_remove (piece_descriptor_t* self) {
  assert(self->prev);
  assert(self->next);

  self->prev->next = self->next;
  self->next->prev = self->prev;
}

piece_descriptor_range_t*
piece_descriptor_range_init (void) {
  piece_descriptor_range_t* self = malloc(sizeof(piece_descriptor_range_t));
  self->is_boundary              = true;
  self->seq_length               = 0;
  self->index                    = 0;
  self->length                   = 0;
  self->first                    = NULL;
  self->last                     = NULL;

  return self;
}

void
piece_descriptor_range_free (piece_descriptor_range_t* self) {
  free(self);
}

void
piece_descriptor_range_append (piece_descriptor_range_t* self, piece_descriptor_t* pd) {
  if (!self->first) {
    self->first = pd;
  } else {
    assert(self->last);

    self->last->next = pd;
    pd->prev         = self->last;
  }

  self->last        = pd;
  self->is_boundary = false;
}

void
piece_descriptor_range_append_range (piece_descriptor_range_t* self, piece_descriptor_range_t* pdr) {
  if (!pdr->is_boundary) {
    if (self->is_boundary) {
      self->first       = pdr->first;
      self->last        = pdr->last;
      self->is_boundary = false;
    } else {
      assert(pdr->first);
      assert(self->last);

      pdr->first->prev = self->last;
      self->last->next = pdr->first;
      self->last       = pdr->last;
    }
  }
}

void
piece_descriptor_range_prepend_range (piece_descriptor_range_t* self, piece_descriptor_range_t* pdr) {
  if (!pdr->is_boundary) {
    if (self->is_boundary) {
      self->first       = pdr->first;
      self->last        = pdr->last;
      self->is_boundary = false;
    } else {
      assert(pdr->last);
      assert(self->first);

      pdr->last->next   = self->first;
      self->first->prev = pdr->last;
      self->first       = pdr->first;
    }
  }
}

void
piece_descriptor_range_as_boundary (piece_descriptor_range_t* self, piece_descriptor_t* before, piece_descriptor_t* after) {
  self->first       = before;
  self->last        = after;
  self->is_boundary = true;
}

piece_table_t*
piece_table_init (void) {
  piece_table_t* self    = malloc(sizeof(piece_table_t));
  self->undo_stack       = event_stack_init();
  self->redo_stack       = event_stack_init();
  self->buffer_list      = array_init();
  self->head             = piece_descriptor_init();
  self->tail             = piece_descriptor_init();
  self->frag_1           = NULL;
  self->frag_2           = NULL;
  self->seq_length       = 0;
  self->add_buffer_id    = 0;
  self->last_event_index = 0;
  self->last_event       = PT_SENTINEL;

  self->head->next       = self->tail;
  self->tail->prev       = self->head;

  return self;
}

void
piece_table_setup (piece_table_t* self, char* piece) {
  unsigned int  length     = piece ? strlen(piece) : 0;
  seq_buffer_t* add_buffer = piece_table_alloc_add_buffer(self, length);
  if (piece) {
    buffer_append(add_buffer->buffer, piece);
  }
  add_buffer->length     = length;

  unsigned int        id = array_size(self->buffer_list) - 1;
  piece_descriptor_t* pd = piece_descriptor_init();
  pd->offset             = 0;
  pd->length             = length;
  pd->id                 = id;
  pd->next               = self->tail;
  pd->prev               = self->head;
  self->head->next       = pd;
  self->tail->prev       = pd;
  self->seq_length       = length;

  piece_table_record_event(self, PT_SENTINEL, 0);
}

void
piece_table_free (piece_table_t* self) {
  event_stack_free(self->undo_stack);
  event_stack_free(self->redo_stack);
  array_free(self->buffer_list, (free_fn*)seq_buffer_free);

  while (self->head) {
    piece_descriptor_t* d = self->head;
    piece_descriptor_free(d);
    self->head = self->head->next;
  }

  while (self->frag_1) {
    piece_descriptor_t* d = self->frag_1;
    piece_descriptor_free(d);
    self->frag_1 = self->frag_1->next;
  }

  free(self);
}

unsigned int
piece_table_size (piece_table_t* self) {
  return self->seq_length;
}

void
piece_table_insert (piece_table_t* self, unsigned int index, char* piece) {
  unsigned int length = strlen(piece);

  assert(index <= self->seq_length);  // TODO:

  piece_descriptor_t* pd;
  unsigned int        pd_index = piece_table_desc_from_index(self, index, &pd);

  unsigned int add_buffer_offset = piece_table_import_buffer(self, piece, length);

  event_stack_clear(self->redo_stack);

  unsigned int insert_offset        = index - pd_index;

  piece_descriptor_range_t* new_pds = piece_descriptor_range_init();

  // Inserting at the end of a prior insertion - at a pd boundary
  if (insert_offset == 0 && piece_table_can_optimize(self, PT_INSERT, index)) {
    assert(pd->prev);
    // Extend the last pd's length
    piece_descriptor_range_t* ev  = event_stack_last(self->undo_stack);
    pd->prev->length             += length;
    ev->length                   += length;

    // Inserting at a pd boundary
  } else if (insert_offset == 0) {
    piece_descriptor_range_t* old_pds = piece_table_undo_range_init(self, index, length);
    piece_descriptor_range_as_boundary(old_pds, pd->prev, pd);

    piece_descriptor_t* pd1 = piece_descriptor_init();
    pd1->length             = length;
    pd1->buffer             = self->add_buffer_id;
    pd1->offset             = add_buffer_offset;

    piece_descriptor_range_append(new_pds, pd1);
    piece_table_swap_desc_ranges(self, old_pds, new_pds);
    // Inserting in the middle of a piece
  } else {
    piece_descriptor_range_t* old_pds = piece_table_undo_range_init(self, index, length);
    piece_descriptor_range_append(old_pds, pd);

    piece_descriptor_t* pd1 = piece_descriptor_init();
    pd1->length             = insert_offset;
    pd1->buffer             = pd->buffer;
    pd1->offset             = pd->offset;
    piece_descriptor_range_append(new_pds, pd1);

    piece_descriptor_t* pd2 = piece_descriptor_init();
    pd2->length             = length;
    pd2->buffer             = self->add_buffer_id;
    pd2->offset             = add_buffer_offset;
    piece_descriptor_range_append(new_pds, pd2);

    piece_descriptor_t* pd3 = piece_descriptor_init();
    pd3->length             = pd->length - insert_offset;
    pd3->buffer             = pd->buffer;
    pd3->offset             = pd->offset + insert_offset;
    piece_descriptor_range_append(new_pds, pd3);

    piece_table_swap_desc_ranges(self, old_pds, new_pds);
  }

  self->seq_length += length;

  piece_table_record_event(self, PT_INSERT, index + length);
}

void
piece_table_delete (piece_table_t* self, unsigned int index, unsigned int length) {
  // TODO:
  assert(length != 0);
  assert(length <= self->seq_length);
  assert(index <= self->seq_length - length);

  piece_descriptor_t* pd;
  unsigned int        pd_index = piece_table_desc_from_index(self, index, &pd);

  unsigned int rm_offset       = index - pd_index;
  unsigned int rm_length       = length;

  bool append_pd_range         = false;

  piece_descriptor_range_t* evr;
  piece_descriptor_range_t* new_pds = piece_descriptor_range_init();
  piece_descriptor_range_t* old_pds = piece_descriptor_range_init();

  // Forward-delete
  if (index == pd_index && piece_table_can_optimize(self, PT_DELETE, index)) {
    evr              = event_stack_back(self->undo_stack, 0);
    evr->length     += length;

    append_pd_range  = true;

    if (self->frag_2) {
      if (length < self->frag_2->length) {
        self->frag_2->length -= length;
        self->frag_2->offset += length;
        self->seq_length     -= length;

        return;
      } else {
        rm_length -= pd->length;
        pd         = pd->next;

        piece_descriptor_remove(self->frag_2);
      }
    }
    // Backward delete
  } else if (index + length == pd_index + pd->length && piece_table_can_optimize(self, PT_DELETE, index + length)) {
    evr              = event_stack_last(self->undo_stack);
    evr->length     += length;
    evr->index      -= index;

    append_pd_range  = false;

    if (self->frag_1) {
      if (length < self->frag_1->length) {
        self->frag_1->length -= length;
        self->frag_1->offset += 0;
        self->seq_length     -= length;
        return;
      } else {
        rm_length -= self->frag_1->length;
        piece_descriptor_remove(self->frag_1);
      }
    }
  } else {
    append_pd_range = true;

    self->frag_1 = self->frag_2 = NULL;

    evr = piece_table_undo_range_init(self, index, length);
  }

  event_stack_clear(self->redo_stack);

  // Deletion starts midway through a piece
  if (rm_offset != 0) {
    piece_descriptor_t* npd = piece_descriptor_init();
    npd->offset             = pd->offset;
    npd->length             = rm_offset;
    npd->buffer             = pd->buffer;
    piece_descriptor_range_append(new_pds, npd);

    self->frag_1 = new_pds->first;

    if (rm_offset + rm_length < pd->length) {
      piece_descriptor_t* npd2 = piece_descriptor_init();
      npd2->offset             = pd->offset + rm_offset + rm_length;
      npd2->length             = pd->length - rm_offset - rm_length;
      npd2->buffer             = pd->buffer;
      piece_descriptor_range_append(new_pds, npd2);

      self->frag_2 = new_pds->last;
    }

    rm_length -= min(rm_length, pd->length - rm_offset);

    piece_descriptor_range_append(old_pds, pd);
    pd = pd->next;
  }

  while (rm_length > 0 && pd != self->tail) {
    if (rm_length < pd->length) {
      piece_descriptor_t* npd = piece_descriptor_init();
      npd->offset             = pd->offset + rm_length;
      npd->length             = pd->length - rm_length;
      npd->buffer             = pd->buffer;
      piece_descriptor_range_append(new_pds, npd);

      self->frag_2 = new_pds->last;
    }

    rm_length -= min(rm_length, pd->length);

    piece_descriptor_range_append(old_pds, pd);
    pd = pd->next;
  }

  piece_table_swap_desc_ranges(self, old_pds, new_pds);
  self->seq_length -= length;

  if (append_pd_range) {
    piece_descriptor_range_append_range(evr, old_pds);
  } else {
    piece_descriptor_range_prepend_range(evr, old_pds);
  }

  piece_table_record_event(self, PT_DELETE, index);
}

piece_descriptor_range_t*
piece_table_undo_range_init (piece_table_t* self, unsigned int index, unsigned int length) {
  piece_descriptor_range_t* undo_range = piece_descriptor_range_init();
  undo_range->seq_length               = self->seq_length;
  undo_range->index                    = index;
  undo_range->length                   = length;

  event_stack_push(self->undo_stack, undo_range);

  return undo_range;
}

void
piece_table_undo (piece_table_t* self) {
  piece_table_do_stack_event(self, self->undo_stack, self->redo_stack);
}

void
piece_table_redo (piece_table_t* self) {
  piece_table_do_stack_event(self, self->redo_stack, self->undo_stack);
}

unsigned int
piece_table_render (piece_table_t* self, unsigned int index, unsigned int length, char* dest) {
  // TODO: cache
  unsigned int total = 0;

  piece_descriptor_t* pd;
  unsigned int        pd_index  = piece_table_desc_from_index(self, index, &pd);
  unsigned int        pd_offset = index - pd_index;

  while (length && (pd && pd != self->tail)) {
    unsigned int copy_len = min(pd->length - pd_offset, length);
    char*        src
      = buffer_state(((seq_buffer_t*)array_get(self->buffer_list, pd->buffer))->buffer);
    unsigned int start = pd->offset + pd_offset;
    unsigned int end   = start + copy_len;

    memcpy(dest, src + pd->offset + pd_offset, copy_len * sizeof(char));

    dest      += copy_len;
    length    -= copy_len;
    total     += copy_len;
    pd         = pd->next;
    pd_offset  = 0;
  }

  if (length == 0) {
    memcpy(dest, "", 1);
  }

  return total;
}

void
piece_table_do_stack_event (piece_table_t* self, event_stack_t* src, event_stack_t* dest) {
  if (event_stack_empty(src)) {
    return;  // TODO:
  }

  piece_table_record_event(self, PT_SENTINEL, 0);

  piece_descriptor_range_t* range;
  do {
    range = event_stack_last(src);
    event_stack_pop(src);
    event_stack_push(dest, range);
    piece_table_restore_desc_ranges(self, range);
  } while (!event_stack_empty(src));
}

seq_buffer_t*
piece_table_alloc_buffer (piece_table_t* self, unsigned int max_size) {
  seq_buffer_t* sb = seq_buffer_init();
  sb->length       = 0;
  sb->max_size     = max_size;
  sb->id           = array_size(self->buffer_list);
  array_push(self->buffer_list, sb);
  return sb;
}

seq_buffer_t*
piece_table_alloc_add_buffer (piece_table_t* self, unsigned int max_size) {
  seq_buffer_t* sb    = piece_table_alloc_buffer(self, max_size);
  self->add_buffer_id = sb->id;
  return sb;
}

unsigned int
piece_table_import_buffer (piece_table_t* self, char* s, unsigned int length) {
  seq_buffer_t* buf = (seq_buffer_t*)array_get(self->buffer_list, self->add_buffer_id);
  if (buf->length + length >= buf->max_size) {
    buf = piece_table_alloc_add_buffer(self, length + 0x10000);
    piece_table_record_event(self, PT_SENTINEL, 0);
  }

  buffer_append(buf->buffer, s);

  unsigned int ret  = buf->length;
  buf->length      += length;

  return ret;
}

void
piece_table_swap_desc_ranges (piece_table_t* self, piece_descriptor_range_t* src, piece_descriptor_range_t* dest) {
  assert(src->first);
  assert(src->last);

  if (src->is_boundary) {
    if (!dest->is_boundary) {
      assert(dest->first);
      assert(dest->last);

      src->first->next  = dest->first;
      src->last->prev   = dest->last;
      dest->first->prev = src->first;
      dest->last->next  = src->last;
    }
  } else {
    assert(src->first->prev);
    assert(src->last->next);

    if (dest->is_boundary) {
      src->first->prev->next = src->last->next;
      src->last->next->prev  = src->first->prev;
    } else {
      assert(dest->first);
      assert(dest->last);

      src->first->prev->next = dest->first;
      src->last->next->prev  = dest->last;
      dest->first->prev      = src->first->prev;
      dest->last->next       = src->last->next;
    }
  }
}

void
piece_table_restore_desc_ranges (piece_table_t* self, piece_descriptor_range_t* pdr) {
  if (pdr->is_boundary) {
    piece_descriptor_t* first = pdr->first->next;
    piece_descriptor_t* last  = pdr->last->prev;

    assert(pdr->first);
    assert(pdr->last);

    // unlink descs from main list
    pdr->first->next = pdr->last;
    pdr->last->prev  = pdr->first;

    // store the desc range we just removed
    pdr->first       = first;
    pdr->last        = last;
    pdr->is_boundary = false;
  } else {
    piece_descriptor_t* first = pdr->first->prev;
    piece_descriptor_t* last  = pdr->last->next;

    assert(first);
    assert(last);

    // are we moving descs into an "empty" region?
    // (i.e. in between two adjacent descs)
    if (first->next == last) {
      // move the old descs back into the empty region
      first->next      = pdr->first;
      last->prev       = pdr->last;

      // store the desc range we just removed
      pdr->first       = first;
      pdr->last        = last;
      pdr->is_boundary = true;
    }
    // we are replacing a range of descs in the list -
    // swap the descs in the list with those in our undo ev
    else {
      // find the desc range that is currently in the list
      first = first->next;
      last  = last->prev;

      assert(first->prev);
      assert(last->next);

      assert(last);

      // unlink the the descs from the main list
      first->prev->next = pdr->first;
      last->next->prev  = pdr->last;

      // store the desc range we just removed
      pdr->first        = first;
      pdr->last         = last;
      pdr->is_boundary  = false;
    }
  }

  unsigned int tmp = pdr->seq_length;
  pdr->seq_length  = self->seq_length;
  self->seq_length = tmp;
}

unsigned int
piece_table_desc_from_index (piece_table_t* self, unsigned int index, piece_descriptor_t** pd) {
  unsigned int curr_index = 0;
  unsigned int pd_index   = 0;

  for (*pd = self->head->next; (*pd)->next; *pd = (*pd)->next) {
    if (index >= curr_index && index < curr_index + (*pd)->length) {
      // if (pd_index) {
      pd_index = curr_index;
      // }

      return pd_index;
    }

    curr_index += (*pd)->length;
  }

  // Insert at tail
  if (*pd && index == curr_index) {
    pd_index = curr_index;
    return pd_index;
  }

  // Should never get here
  assert(false);
}

void
piece_table_record_event (piece_table_t* self, piece_table_event ev, unsigned int index) {
  self->last_event       = ev;
  self->last_event_index = index;
}

bool
piece_table_can_optimize (piece_table_t* self, piece_table_event ev, unsigned int index) {
  return self->last_event == ev && self->last_event_index == index;
}
