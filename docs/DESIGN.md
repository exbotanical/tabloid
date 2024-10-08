# Design Docs and Notes

## Renderbuf

   0 1 2 3 4
0 |G|R|E|E|T|
1 |I|N|G|S| |
2 | | | | | |
3 |D|U|D|E| |
4 | | | | | |
5 | | | | | |
6 | | | | | |
7 | | | | | |
8 | | | | | |
9 | | | | | |

y=0
x=8
raw_buffer = [
  state=['G', 'R', 'E', 'E', 'T', 'I', 'N', 'G', 'S'],
  state=[],
  state=['D', 'U', 'D', 'E']
]

y=1
x=4

y=2
x=4
render_buffer = [
  state=['G', 'R', 'E', 'E', 'T'], rowref=0
  state=['I', 'N', 'G', 'S', '\n'], rowref=0,
  state=[], rowref=1,
  state=['D', 'U', 'D', 'E'], rowref=2,
]
