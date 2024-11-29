#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include <stdbool.h>

#include "libutil/libutil.h"

typedef enum {
  PT_SENTINEL,
  PT_INSERT,
  PT_DELETE,
  PT_REPLACE,
} piece_table_event;

typedef struct {
  /// array_t<piece_descriptor_range*>
  array_t* event_captures;
} event_stack_t;

typedef struct {
  unsigned int length;
  unsigned int max_size;
  unsigned int id;
  buffer_t*    buffer;
} seq_buffer_t;

typedef struct piece_descriptor piece_descriptor_t;

typedef struct piece_descriptor {
  unsigned int        id;
  unsigned int        offset;
  unsigned int        length;
  unsigned int        buffer;
  piece_descriptor_t* next;
  piece_descriptor_t* prev;
} ___piece_descriptor_t;

typedef struct {
  bool                is_boundary;
  unsigned int        seq_length;
  unsigned int        index;
  unsigned int        length;
  unsigned int        group_id;
  piece_descriptor_t* first;
  piece_descriptor_t* last;
  void*               metadata;
} piece_descriptor_range_t;

typedef struct {
  event_stack_t*      undo_stack;
  event_stack_t*      redo_stack;
  piece_descriptor_t* frag_1;
  piece_descriptor_t* frag_2;
  piece_descriptor_t* head;
  piece_descriptor_t* tail;
  unsigned int        seq_length;
  unsigned int        add_buffer_id;
  unsigned int        last_event_index;
  /// array_t<seq_buffer_t*>
  array_t*            buffer_list;
  piece_table_event   last_event;
  int                 offset_since_dirty_reset;
} piece_table_t;

seq_buffer_t* seq_buffer_init(void);
void          seq_buffer_free(seq_buffer_t* self);

event_stack_t*            event_stack_init(void);
void                      event_stack_free(event_stack_t* self);
bool                      event_stack_empty(event_stack_t* self);
piece_descriptor_range_t* event_stack_last(event_stack_t* self);
void                      event_stack_push(event_stack_t* self, piece_descriptor_range_t* pdr);
piece_descriptor_range_t* event_stack_pop(event_stack_t* self);
void                      event_stack_clear(event_stack_t* self);
piece_descriptor_range_t* event_stack_back(event_stack_t* self, unsigned int index);

piece_descriptor_t* piece_descriptor_init(void);
void                piece_descriptor_free(piece_descriptor_t* self);
void                piece_descriptor_remove(piece_descriptor_t* self);

piece_descriptor_range_t* piece_descriptor_range_init(void);
void                      piece_descriptor_range_free(piece_descriptor_range_t* self);
void piece_descriptor_range_append(piece_descriptor_range_t* self, piece_descriptor_t* pd);
void piece_descriptor_range_append_range(piece_descriptor_range_t* self, piece_descriptor_range_t* pdr);
void piece_descriptor_range_prepend_range(piece_descriptor_range_t* self, piece_descriptor_range_t* pdr);
void piece_descriptor_range_as_boundary(piece_descriptor_range_t* self, piece_descriptor_t* before, piece_descriptor_t* after);

piece_table_t* piece_table_init(void);
void           piece_table_setup(piece_table_t* self, char* piece);
void           piece_table_free(piece_table_t* self);
unsigned int   piece_table_size(piece_table_t* self);

void piece_table_insert(piece_table_t* self, unsigned int index, char* piece, void* metadata);
void piece_table_delete(piece_table_t* self, unsigned int index, unsigned int length, piece_table_event ev, void* metadata);
void* piece_table_undo(piece_table_t* self);
piece_descriptor_range_t* piece_table_undo_range_init(piece_table_t* self, unsigned int index, unsigned int length, void* metadata);
void* piece_table_redo(piece_table_t* self);

unsigned int piece_table_render(piece_table_t* self, unsigned int index, unsigned int length, char* dest);
void* piece_table_do_stack_event(piece_table_t* self, event_stack_t* src, event_stack_t* dest);
seq_buffer_t* piece_table_alloc_buffer(piece_table_t* self, unsigned int max_size);
seq_buffer_t* piece_table_alloc_add_buffer(piece_table_t* self, unsigned int max_size);
unsigned int  piece_table_import_buffer(piece_table_t* self, char* s, unsigned int length);
void piece_table_swap_desc_ranges(piece_table_t* self, piece_descriptor_range_t* src, piece_descriptor_range_t* dest);
void piece_table_restore_desc_ranges(piece_table_t* self, piece_descriptor_range_t* pdr);
unsigned int piece_table_desc_from_index(piece_table_t* self, unsigned int index, piece_descriptor_t** pd);

void piece_table_record_event(piece_table_t* self, piece_table_event ev, unsigned int index);
bool piece_table_can_optimize(piece_table_t* self, piece_table_event ev, unsigned int index);
void piece_table_break(piece_table_t* self);
bool piece_table_dirty(piece_table_t* self);
void piece_table_dirty_reset(piece_table_t* self);

#endif /* PIECE_TABLE_H */
