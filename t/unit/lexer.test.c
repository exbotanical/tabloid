#include "lexer.h"

#include "tests.h"

typedef struct {
  array_t* expect;
  char     in[64];
} test_case;

token_t*
make_token (token_type_t type, char* value) {
  token_t* tok = malloc(sizeof(tok));
  tok->type    = type;
  tok->value   = value;

  return tok;
}

static void
test_lexer_basic (void) {
  token_t* str_token = make_token(TOKEN_STRING, "tok");

  /* clang-format off */
  test_case test_cases[] = {
    {
      .in     = "  tok",
      .expect = array_collect(str_token),
    },
    {
      .in     = "  tok !",
      .expect = array_collect(str_token, make_token(TOKEN_STRING, "!")),
    },
    {
      .in     = "  tok!!",
      .expect = array_collect(make_token(TOKEN_STRING, "tok!!")),
    },
    {
      .in     = "  tok tok",
      .expect = array_collect(str_token, str_token),
    },
    {
      .in     = "  tok ! tok",
      .expect = array_collect(str_token, make_token(TOKEN_STRING, "!"), str_token),
    },
    {
      .in     = "  tok!! tok",
      .expect = array_collect(make_token(TOKEN_STRING, "tok!!"), str_token),
    },
    {
      .in     = "tok",
      .expect = array_collect(str_token),
    },
    {
      .in     = "tok !",
      .expect = array_collect(str_token, make_token(TOKEN_STRING, "!")),
    },
    {
      .in     = "tok!!",
      .expect = array_collect(make_token(TOKEN_STRING, "tok!!")),
    },
    {
      .in     = "tok tok",
      .expect = array_collect(str_token, str_token),
    },
    {
      .in     = "tok ! tok",
      .expect = array_collect(str_token, make_token(TOKEN_STRING, "!"), str_token),
    },
    {
      .in     = "tok!! tok",
      .expect = array_collect(make_token(TOKEN_STRING, "tok!!"), str_token),
    },
    {
      .in     = "!",
      .expect = array_collect(make_token(TOKEN_STRING, "!")),
    },
    {
      .in     = "  !",
      .expect = array_collect(make_token(TOKEN_STRING, "!")),
    },
    {
      .in     = "    ",
      .expect = array_collect(NULL),
    },
    {
      .in = " w! 'name'",
      .expect = array_collect(make_token(TOKEN_STRING, "w!"), make_token(TOKEN_STRING, "'name'")),
    },
    {
      .in = " w! name",
      .expect = array_collect(make_token(TOKEN_STRING, "w!"), make_token(TOKEN_STRING, "name")),
    },
  };

  /* clang-format on |*/

  lexer_t lexer;

  FOR_EACH_TEST({
    lexer_init(&lexer, tc.in);
    array_t* tokens = lexer_tokenize(&lexer);

    foreach (tc.expect, i) {
      token_t* expect = (token_t*)array_get(tc.expect, i);
      token_t* actual = (token_t*)array_get(tokens, i);

      if (!expect) {
        ok(actual == NULL, "has no tokens");
        continue;
      }

      ok(expect->type == actual->type, "has expected token type");
      is(expect->value, actual->value, "has expected token value");
    }

    free(tokens);
  })
}

void
run_lexer_tests (void) {
  test_lexer_basic();
}
