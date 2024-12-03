#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "xmalloc.h"

#define IF_COMMAND(str, cmd)         \
  if (s_equals(token->value, str)) { \
    ct->command = cmd;               \
    ret         = true;              \
  }

#define SET_ERROR(str)             \
  {                                \
    ct->command = COMMAND_INVALID; \
    ct->error   = str;             \
    ret         = false;           \
  }

command_token_t*
parser_command_token_init (void) {
  command_token_t* ct = xmalloc(sizeof(command_token_t));

  ct->mods            = 0;
  ct->error           = NULL;
  ct->arg             = NULL;

  return ct;
}

void
parser_command_token_free (command_token_t* self) {
  if (self->command != COMMAND_INVALID && self->arg) {
    free(self->arg);
  }
  free(self);
}

void
parser_init (parser_t* self) {
  self->cursor = 0;
}

static bool
parser_parse_command (parser_t* self, array_t* tokens, command_token_t* ct) {
  bool ret          = true;

  token_t* token    = (token_t*)array_get(tokens, 0);
  bool     has_args = array_size(tokens) > 1;
  size_t   len      = strlen(token->value);

  if (token->value[len - 1] == '!') {
    token->value[len - 1]  = '\0';
    ct->mods              |= TOKEN_MOD_OVERRIDE;
  }

  IF_COMMAND("w", COMMAND_WRITE)
  IF_COMMAND("q", COMMAND_QUIT)
  IF_COMMAND("wq", COMMAND_WRITE_QUIT)

  switch (ct->command) {
    case COMMAND_WRITE:
    case COMMAND_WRITE_QUIT: {
      if (has_args) {
        buffer_t* buf = buffer_init(NULL);
        foreach_i(tokens, i, 1) {
          token_t* ctoken = (token_t*)array_get(tokens, i);

          if (ctoken->type != TOKEN_STRING) {
            SET_ERROR("invalid type");
            break;
          }

          buffer_append(buf, ctoken->value);
        }

        ct->arg = s_copy(buffer_state(buf));

        buffer_free(buf);

        break;
      }
    }

    case COMMAND_QUIT: {
      if (has_args) {
        SET_ERROR("trailing text");
      }
      break;
    }

    default: {
      SET_ERROR("unknown command");
      break;
    }
  }

  return ret;
}

// TODO: ???
static void
parser_parse_argument (parser_t* self, token_t* token) {}

void
parser_parse (parser_t* self, array_t* tokens, command_token_t* command) {
  bool has_args = array_size(tokens) > 0;

  parser_parse_command(self, tokens, command);

  foreach (tokens, i) {
    free(((token_t*)array_get(tokens, i))->value);
  }

  array_free(tokens, free);
}

command_token_t*
parser_parse_wrapped (const char* source) {
  parser_t parser;
  lexer_t  lexer;

  lexer_init(&lexer, source);
  parser_init(&parser);

  command_token_t* command = parser_command_token_init();
  parser_parse(&parser, lexer_tokenize(&lexer), command);

  return command;
}
