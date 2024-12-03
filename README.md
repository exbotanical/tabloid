# Tabloid

A minimalist, extensible text editor.

Implemented using a piece table data structure, lexer & parser for commands, Knuth–Morris–Pratt search.

Features:

- Discrete Command and Edit modes
  - Commands (end with `!` for override):
    - Write `w` <?filepath>
    - Write-quit `wq`
    - Quit `q`
- Highlight and select
  - Highlight: shift+arrow
  - Highlight word: ctrl+shift+arrow
- Keybindings for:
  - Delete line preceding cursor: ctrl+u
  - Jump to line begin: ctrl+a, home
  - Jump to line end: ctrl+e, end
  - Undo: ctrl+z
