#ifndef LEXER_H
#define LEXER_H

#include "libutil/libutil.h"
#include "scanner.h"

typedef enum {
  TOKEN_STRING,
} token_type_t;

typedef struct {
  token_type_t type;
  char*        value;
} token_t;

typedef struct {
  scanner_t scanner;
} lexer_t;

void     lexer_init(lexer_t* self, const char* source);
array_t* lexer_tokenize(lexer_t* self);

#endif /* LEXER_H */
