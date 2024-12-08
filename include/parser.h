#ifndef PARSER_H
#define PARSER_H

#include "libutil/libutil.h"
#include "utils.h"

typedef enum {
  TOKEN_MOD_OVERRIDE = 1,
} token_modifier_t;

typedef enum {
  COMMAND_WRITE = 1,
  COMMAND_QUIT,
  COMMAND_WRITE_QUIT,

  PCOMMAND_SEARCH,

  COMMAND_INVALID,
} command_t;

static const char* command_enum_displays[] = {
  X(COMMAND_WRITE),
  X(COMMAND_QUIT),
  X(COMMAND_WRITE_QUIT),

  X(PCOMMAND_SEARCH),

  X(COMMAND_INVALID),
};

typedef struct {
  int mods;

  // TODO: Size limit
  union {
    char* arg;
    char* error;
  };

  command_t command;
} command_token_t;

typedef struct {
  int cursor;
} parser_t;

command_token_t* parser_command_token_init(void);
void             parser_command_token_free(command_token_t* self);

void             parser_init(parser_t* self);
void             parser_parse(parser_t* self, array_t* tokens, command_token_t* command);
command_token_t* parser_parse_wrapped(const char* source);

#endif /* PARSER_H */
