# TODOs
- [ ] status bar
- [ ] file I/O
  - [ ] swap files
- [ ] history (undo/redo stack)
- [ ] command mode
- [ ] keybindings config support
  - [ ] default keybindings/config
- [ ] LSP support
- [x] line numbers
- [ ] terminfo
- [ ] modes (editor, command)
- [ ] plugins system
  - [ ] git
  - [ ] search
- [x] fix scroll out / capture window
- [ ] bounded / max size guards (e.g. max file size or max num lines - temporary)
- [x] unit tests
- [ ] integ tests
- [x] current row highlight
- [ ] handle screen size change
- [ ] handle wrap on out-of-bounds long line (e.g. wrap around but maintain lineno)
  - [ ] OK doing this is REALLY fucking difficult. Do it later.
    - [ ] Double note: So I read the vim and neovim source code and holy fuck the implementation is bonkers. They essentially implement a virtualization layer (yes, like paging in an operating system) and then manage blocks of text buffers underneath the UI. Code is too cryptic to easily discern how they handle the cursor logic on top of this. Do this later.
- [ ] Keybindings:
  - [ ] cut
  - [ ] copy
    - [ ] copy full line preceding cursor when no select
  - [ ] paste
  - [ ] delete (ctrl+k)
  - [ ] delete word (ctrl+backspace)
  - [ ] select char
  - [ ] select word
  - [ ] select row
  - [ ] select all
  - [ ] select all matches
  - [ ] select match sequential (e.g. ctrl+d ++)
  - [ ] move row (ctrl up/down)
  - [ ] duplicate row (binding -> ???)
  - [ ] ctrl+u delete entire line
- [ ] Multi-cursor editing
  - [ ] dupe cursor up/down
- [x] Explore gap buffer, piece table, or rope for optimized storage
  - [ ] OK let's do a piece table!
- [ ] unicode!
