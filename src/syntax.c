/**
 * @file syntax.c
 * @author goldmund
 * @brief Syntax-highlighting module
 * @version 0.1
 * @date 2021-07-05
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "syntax.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

/* file type */

#define HL_HIGHLIGHT_NUMBERS (1 << 0)  //*< bit flag for numbers */
#define HL_HIGHLIGHT_STRINGS (1 << 1)  //*< bit flag for strings */

/* config for clang rules */
char *C_HL_EXTENSIONS[] = {".c", ".h", NULL};

char *C_HL_KEYWORDS[] = {
    // keyword1
    "switch", "if", "else", "while", "for", "break", "continue", "return",
    "struct", "union", "typedef", "static", "enum", "case", "noreturn",
    "extern", "inline",

    // keyword2
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", "size_t|",

    NULL};

SyntaxConfig HLDB[] = {
    // clang
    {.f_type = "c",
     .f_match = C_HL_EXTENSIONS,
     .keywords = C_HL_KEYWORDS,
     .single_ln_comment_begin = "//",
     .multi_ln_comment_begin = "/*",
     .multi_ln_comment_end = "*/",
     .flags = HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

/**
 * @brief Determine if a character is a delimiter such that it
 * separates a number, indicating the number is not part of a variable or type
 * name e.g. `int16_t`
 *
 *
 * @param c
 * @return int
 */
int is_delimiter(int c) {
  return isspace(c) || c == NULL_TERMINATOR ||
         strchr(",.()+-/*%<>[];", c) != NULL;
}

/**
 * @brief Resolves the current file type to its corresponding entry
 * in the HL database; if not match, syntax is set to `NULL`
 *
 */
void syntax_resolve(void) {
  T.syntax = NULL;

  if (!T.filename) return;

  char *ext = strchr(T.filename, '.');

  for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
    SyntaxConfig *s = &HLDB[j];
    unsigned int i = 0;

    // iter ea hldb entry; check for match; set syntax accordingly
    while (s->f_match[i]) {
      int is_ext = (s->f_match[i][0] == '.');

      if ((is_ext && ext && !strcmp(ext, s->f_match[i])) ||
          (!is_ext && strstr(T.filename, s->f_match[i]))) {
        T.syntax = s;

        // re-highlight all text; this ensures a file that acquires a file type
        // only after being saved is then highlighted
        int f_row;
        for (f_row = 0; f_row < T.num_rows; f_row++) {
          syntax_highlight(&T.row[f_row]);
        }

        return;
      }

      i++;
    }
  }
}

/**
 * @brief Read in a row and for each character, set its highlight / syntax type
 *
 * @param row
 */
void syntax_highlight(Row *row) {
  // where `row.highlight` will either be a NULL pointer
  // or larger than when last passed in

  // we use `rsize` because both arrays are synced
  // and thereby have the same size
  row->highlight = realloc(row->highlight, row->rsize);
  memset(row->highlight, HL_DEFAULT, row->rsize);

  if (!T.syntax) {
    return;
  }

  // pull keyword matches
  char **keywords = T.syntax->keywords;

  // alias single-line comment begin char
  char *single_begin = T.syntax->single_ln_comment_begin;
  // alias multi-line comment begin / end chars
  char *multi_begin = T.syntax->multi_ln_comment_begin;
  char *multi_end = T.syntax->multi_ln_comment_end;

  // pull len of comment char(s); this will be used as a boolean
  // that informs us whether we should HL the comment
  int single_begin_len = single_begin ? strlen(single_begin) : 0;
  int multi_begin_len = multi_begin ? strlen(multi_begin) : 0;
  int multi_end_len = multi_end ? strlen(multi_end) : 0;

  // track the preceding character to the currents
  // set to `true` (1) because beginning of line is indeed a delimiter
  int prev_delimited = 1;
  // track whether we are parsing a string
  int in_str = 0;
  // track whether we are parsing a multi-ln comment
  // `true` if the prev row has an unclosed ml comment i.e. `hl_open_comment` ==
  // 1
  int in_comment = (row->idx > 0 && T.row[row->idx - 1].hl_open_comment);

  int i = 0;
  while (i < row->size) {
    char c = row->render[i];
    unsigned char prev_ch = (i > 0) ? row->highlight[i - 1] : HL_DEFAULT;

    /* single-line comments */
    if (single_begin_len && !in_str &&
        !in_comment /* don't hl comment inside of ml comment */) {
      if (!strncmp(&row->render[i], single_begin, single_begin_len)) {
        memset(&row->highlight[i], HL_COMMENT, row->rsize - i);

        break;
      }
    }

    /* multi-line comments */
    if (multi_begin_len && multi_end_len && !in_str) {
      if (in_comment) {
        row->highlight[i] = HL_ML_COMMENT;

        // are we at the end of a ml comment?
        if (!strncmp(&row->render[i], multi_end, multi_end_len)) {
          memset(&row->highlight[i], HL_ML_COMMENT, multi_end_len);

          i += multi_end_len;
          in_comment = 0;
          prev_delimited = 1;

          continue;
        } else {  // in comment, continue
          i++;

          continue;
        }
        // are we at the beginning of a ml comment?
      } else if (!strncmp(&row->render[i], multi_begin, multi_begin_len)) {
        memset(&row->highlight[i], HL_ML_COMMENT, multi_begin_len);
        i += multi_begin_len;
        in_comment = 1;

        continue;
      }
    }

    /* strings */
    if (T.syntax->flags & HL_HIGHLIGHT_STRINGS) {
      // in a str, highlights
      if (in_str) {
        row->highlight[i] = HL_STRING;

        // ignore escaped quotes
        if (c == '\\' && i + 1 < row->rsize) {
          row->highlight[i + 1] = HL_STRING;
          i += 2;

          continue;
        }

        // check if closing quote
        if (c == in_str) in_str = 0;
        i++;
        // closing quote is a delimiter
        prev_delimited = 1;

        continue;
      } else {
        // hl both dbl and sgl quote strs
        if (c == '"' || c == '\'') {
          // store quote type in `in_str` so we know what will close it
          in_str = c;
          row->highlight[i] = HL_STRING;
          i++;

          continue;
        }
      }
    }

    /* numbers */
    // should numbers be highlighted for the given file type?
    if (T.syntax->flags & HL_HIGHLIGHT_NUMBERS) {
      if ((isdigit(c) && (prev_delimited || prev_ch == HL_NUMBER)) ||
          // support decimal numbers
          (c == '.' && prev_ch == HL_NUMBER)) {
        row->highlight[i] = HL_NUMBER;
        i++;
        prev_delimited = 0;
        continue;
      }
    }

    /* keywords */

    // kw requires a delimiter before and after
    // e.g. we want to prevent highlighting 'avoid' as `void`
    if (prev_delimited) {
      int j;
      for (j = 0; keywords[j]; j++) {
        int kw_len = strlen(keywords[j]);
        int is_kw2 = keywords[j][kw_len - 1] == '|';

        // now that `is_kw2` flag is set, remove the terminating pipe char
        if (is_kw2) kw_len--;

        // check if kw match + delimiter succeeds it

        // TODO should we check only strncmp == 0?
        if (!strncmp(&row->render[i], keywords[j], kw_len) &&
            is_delimiter(row->render[i + kw_len])) {
          // highlight the keyword
          memset(&row->highlight[i], is_kw2 ? HL_KEYWORD2 : HL_KEYWORD1,
                 kw_len);
          i += kw_len;

          break;
        }
      }

      // check if the inner kw iteration reached the terminating `NULL` value
      if (keywords[j] != NULL) {
        prev_delimited = 0;

        continue;
      }
    }

    prev_delimited = is_delimiter(c);
    i++;
  }

  // are we now in a ml comment?
  // e.g. user comments out entire file
  int changed = (row->hl_open_comment != in_comment);

  // set the new ml comment state
  row->hl_open_comment = in_comment;

  // as long as ml comment state is open, we continue updating
  if (changed && row->idx + 1 < T.num_rows) {
    syntax_highlight(&T.row[row->idx + 1]);
  }
}

/**
 * @brief
 *
 * @param hl
 * @return int
 *
 * @todo read configuration file for color settings
 */
int syntax_map_to_color(int hl) {
  switch (hl) {
    case HL_COMMENT:
    case HL_ML_COMMENT:
      return 36;
    case HL_KEYWORD1:
      return 33;
    case HL_KEYWORD2:
      return 32;
    case HL_STRING:
      return 35;
    case HL_NUMBER:
      return 31;
    case HL_SEARCH_MATCH:
      return 34;
    default:
      return 37;
  }
}
