// # Design Docs and Notes

// ## Renderbuf

//    0 1 2 3 4
// 0 |G|R|E|E|T|
// 1 |I|N|G|S| |
// 2 | | | | | |
// 3 |D|U|D|E| |
// 4 | | | | | |
// 5 | | | | | |
// 6 | | | | | |
// 7 | | | | | |
// 8 | | | | | |
// 9 | | | | | |

// y=0
// x=8
// raw_buffer = [
//   state=['G', 'R', 'E', 'E', 'T', 'I', 'N', 'G', 'S'],
//   state=[],
//   state=['D', 'U', 'D', 'E']
// ]

// y=1
// x=4

// y=2
// x=4
// render_buffer = [
//   state=['G', 'R', 'E', 'E', 'T'], rowref=0
//   state=['I', 'N', 'G', 'S', '\n'], rowref=0,
//   state=[], rowref=1,
//   state=['D', 'U', 'D', 'E'], rowref=2,
// ]

// Simulator
const SEPARATOR =
  '------------------------------------------------------------------------------------'

interface RenderRowState {
  linebufRef: number
  offset: number
  state: string[]
}

interface LineBuf {
  state: string[][]
}

interface RenderBuf {
  rows: RenderRowState[]
}

const PAD_OFFSET = 3
const NUM_COLS_IN_ROW = 5
const INPUT = 'HELLO WORLD\n WHAT UP DOGGUS\n MOGGUS\nFROGGUS'

;(async (input: string) => {
  const raw: LineBuf = {
    state: [],
  }

  const render: RenderBuf = {
    rows: [],
  }

  let x = 0
  let y = 0

  let rx = 0
  let ry = 0

  for (const char of input) {
    if (char == '\n') {
      y++
      x = 0
    }
    if (!raw.state[y]) raw.state[y] = []
    x++
    raw.state[y].push(char)
  }

  // window_draw_rows
  for (let idx = 0; idx < raw.state.length; idx++) {
    let offset = 0

    const buf = raw.state[idx]

    // window_draw_row
    if (!render.rows[ry]) {
      render.rows[ry] = {
        linebufRef: idx,
        offset,
        state: [`${idx + 1}  `],
      }
    }

    if (buf.length > NUM_COLS_IN_ROW) {
      for (const char of buf) {
        if (rx == NUM_COLS_IN_ROW) {
          offset++
          ry++
          rx = 0
        }

        if (!render.rows[ry]) {
          render.rows[ry] = {
            linebufRef: idx,
            offset,
            state: [' ', ' ', ' '],
          }
        }

        render.rows[ry].state.push(char)

        rx++
      }
    } else {
      render.rows[ry].state.push(...buf)
    }

    rx = 0
    ry++
  }

  for (const buf of render.rows) {
    console.log(buf.state.filter(s => s !== '\n').join(''))
  }

  edit(render, raw, 2, 1, 'x')
  edit(render, raw, 4, 5, 'x')

  console.log(SEPARATOR)
  for (const buf of render.rows) {
    console.log(buf.state.filter(s => s !== '\n').join(''))
  }
  console.log(SEPARATOR)
  console.log({ raw: raw.state })
})(INPUT)

function edit(
  render: RenderBuf,
  raw: LineBuf,
  x: number,
  y: number,
  char: string,
) {
  const editx = x + PAD_OFFSET
  const edity = y

  const row = render.rows[edity]
  row.state[editx] = char

  const rawr = raw.state[row.linebufRef]
  const realx = editx - PAD_OFFSET + row.offset * NUM_COLS_IN_ROW

  rawr[realx] = char
}
