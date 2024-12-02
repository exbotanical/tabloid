#include "parser.h"

#include "lexer.h"
#include "tests.h"

typedef struct {
  char*     in;
  char*     error;
  char*     arg;
  command_t command;
  bool      override;
} test_case;

static void
test_parser_basic (void) {
  test_case test_cases[] = {
    {.in = "w!", .command = WRITE, .arg = NULL, .error = NULL, .override = true},
    {.in = "w", .command = WRITE, .arg = NULL, .error = NULL, .override = false},
    {.in = "w! hello", .command = WRITE, .arg = "hello", .error = NULL, .override = true},

    {.in = "wq!", .command = WRITE_QUIT, .arg = NULL, .error = NULL, .override = true},
    {.in = "wq", .command = WRITE_QUIT, .arg = NULL, .error = NULL, .override = false},
    {.in = "wq! hello", .command = WRITE_QUIT, .arg = "hello", .error = NULL, .override = true},

    {.in = "q!", .command = QUIT, .arg = NULL, .error = NULL, .override = true},
    {.in = "q", .command = QUIT, .arg = NULL, .error = NULL, .override = false},
    {.in = "q! hello", .command = INVALID, .arg = NULL, .error = "trailing text"},
  };

  /* clang-format off */
  FOR_EACH_TEST({
    command_token_t* command = parser_parse_wrapped (tc.in);

    if (tc.error) {
      is(
        command->error,
        tc.error,
        "has the expected error message (actual=%s, expected=%s)",
        command->error,
        tc.error
      );
    } else {
      is(
        command->arg, tc.arg,
        "has expected arg (actual=%s, expected=%s)",
        command->arg,
        tc.arg
      );
      ok(
        (command->mods & TOKEN_MOD_OVERRIDE) == TOKEN_MOD_OVERRIDE == tc.override,
        "has the expected override mod value"
      );
    }

    ok(
      command->command == tc.command,
      "has the correct command (actual=%s, expected=%s)",
      command_enum_displays[command->command],
      command_enum_displays[tc.command]
    );

    parser_command_token_free(command);
  });
  /* clang-format on */
}

void
run_parser_tests (void) {
  test_parser_basic();
}
