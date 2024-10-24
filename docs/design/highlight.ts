// This is a rough scratchpad for working out the design of text selection/highlighting
const HIGHLIGHT_CHAR = 'H'
const NUM_COLS = 10
const NUM_ROWS = 5

interface CursorState {
  x: number
  y: number
}

interface HighlightState {
  active: boolean
  offset: CursorState
  anchor: CursorState
}

class CursorManager {
  cursor: CursorState = {
    x: 0,
    y: 0,
  }
  hl: HighlightState = {
    active: false,
    offset: {
      x: 0,
      y: 0,
    },
    anchor: {
      x: 0,
      y: 0,
    },
  }

  setCursor(x: number, y: number) {
    this.cursor.x = x
    this.cursor.y = y
  }

  moveLeft() {
    if (this.cursor.x == 0) {
      if (this.cursor.y === 0) return
      this.setCursor(NUM_COLS - 1, this.cursor.y - 1)
    } else {
      this.cursor.x--
    }
  }

  moveRight() {
    if (this.cursor.x == NUM_COLS - 1) {
      this.setCursor(0, this.cursor.y + 1)
    } else {
      this.cursor.x++
    }
  }

  highlightRight() {
    if (!this.hl.active) {
      Object.assign(this.hl.anchor, { x: this.cursor.x, y: this.cursor.y })
    }
    this.hl.active = true
    this.moveRight()
    Object.assign(this.hl.offset, { x: this.cursor.x, y: this.cursor.y })
  }

  highlightLeft() {
    if (!this.hl.active) {
      Object.assign(this.hl.anchor, { x: this.cursor.x, y: this.cursor.y })
    }
    this.hl.active = true
    this.moveLeft()
    Object.assign(this.hl.offset, { x: this.cursor.x, y: this.cursor.y })
  }

  isHighlightLtr() {
    return (
      this.hl.anchor.y < this.hl.offset.y ||
      (this.hl.anchor.y === this.hl.offset.y &&
        this.hl.anchor.x <= this.hl.offset.x)
    )
  }

  clear() {
    Object.assign(this.cursor, {
      x: 0,
      y: 0,
    })
    Object.assign(this.hl, {
      active: false,
      offset: {
        x: -1,
        y: -1,
      },
      anchor: {
        x: -1,
        y: -1,
      },
    })
  }
}

class Renderer {
  writeState: string = ''
  constructor(
    private readonly cm: CursorManager,
    private readonly gridConfig: { rows: number; cols: number },
    private readonly lineBuffer: string[],
  ) {}

  flush() {
    this.writeState = ''
  }

  write(char: string) {
    this.writeState += char
    process.stdout.write(char)
  }

  draw() {
    this.flush()

    const isLtr = this.cm.isHighlightLtr()
    // console.log({
    //   isLtr,
    //   ...this.cm.cursor,
    //   ...this.cm.hl,
    // })

    for (let i = 0; i < this.gridConfig.rows; i++) {
      for (let j = 0; j < this.gridConfig.cols; j++) {
        const maybeChar = this.lineBuffer[i]?.[j]
        if (maybeChar) {
          if (this.cm.hl.active) {
            if (isLtr) {
              // If multiple lines are highlighted, and this line falls between the anchor and offset lines,
              // we just highlight the entire line
              if (i > this.cm.hl.anchor.y && i < this.cm.hl.offset.y) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              // If the anchor and offset are on the same line,
              // just highlight between the anchor x and offset x
              if (i === this.cm.hl.anchor.y && i === this.cm.hl.offset.y) {
                if (j >= this.cm.hl.anchor.x && j < this.cm.hl.offset.x) {
                  this.write(HIGHLIGHT_CHAR)
                  continue
                } else {
                  this.write(maybeChar)
                  continue
                }
              }

              // If we're on the anchor line, we highlight from the anchor x onward
              if (i === this.cm.hl.anchor.y && j >= this.cm.hl.anchor.x) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              // If we're on the offset line, we highlight until the the anchor x
              if (i === this.cm.hl.offset.y && j < this.cm.hl.offset.x) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              this.write(maybeChar)
              continue
            } else {
              if (i < this.cm.hl.anchor.y && i > this.cm.hl.offset.y) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              if (i === this.cm.hl.anchor.y && i === this.cm.hl.offset.y) {
                if (j < this.cm.hl.anchor.x && j >= this.cm.hl.offset.x) {
                  this.write(HIGHLIGHT_CHAR)
                  continue
                } else {
                  this.write(maybeChar)
                  continue
                }
              }

              if (i === this.cm.hl.anchor.y && j < this.cm.hl.anchor.x) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              if (i === this.cm.hl.offset.y && j >= this.cm.hl.offset.x) {
                this.write(HIGHLIGHT_CHAR)
                continue
              }

              this.write(maybeChar)
              continue
            }
          } else {
            this.write(maybeChar)
          }
        } else {
          this.write('.')
        }
      }
      this.write('\n')
    }
  }
}

if (import.meta.vitest) {
  const { describe, test, expect, beforeEach } = import.meta.vitest

  const cm = new CursorManager()
  const r = new Renderer(cm, { rows: NUM_ROWS, cols: NUM_COLS }, [
    'fishtanksh',
    'worldstars',
    'whataburgr',
    'dawg',
  ])

  function drawAndAssert(assertWriteStateIs: string) {
    r.draw()
    expect(r.writeState).toEqual(assertWriteStateIs)
  }

  describe('tests', () => {
    beforeEach(() => cm.clear())

    test.skip('test', () => {
      drawAndAssert(
        'fishtanksh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.setCursor(8, 0)
      cm.highlightRight()
      drawAndAssert(
        'fishtankHh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      drawAndAssert(
        'fishtankHH\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      drawAndAssert(
        'fishtankHH\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      drawAndAssert(
        'fishtankHH\nHHrldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      drawAndAssert(
        'fishtankHH\nHHHldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtankHH\nHHrldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtankHH\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtankHH\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtankHh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtanksh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtanHsh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtaHHsh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )
    })

    test('test2', () => {
      drawAndAssert(
        'fishtanksh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.setCursor(1, 1)
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      drawAndAssert(
        'fishtanksh\nwHHHHHHHHH\nHHataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      drawAndAssert(
        'fishtanksh\nwHHHHHHHHs\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      drawAndAssert(
        'fishtanksh\nworldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtanksh\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      drawAndAssert(
        'fishtanksH\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      drawAndAssert(
        'fishHHHHHH\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      cm.highlightLeft()
      // TODO: boundaries
      drawAndAssert(
        'HHHHHHHHHH\nHorldstars\nwhataburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      drawAndAssert(
        'fishtanksh\nwHHHHHHHHH\nHHHHHburgr\ndawg......\n..........\n',
      )

      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      cm.highlightRight()
      drawAndAssert(
        'fishtanksh\nwHHHHHHHHH\nHHHHHHHHHH\nHHwg......\n..........\n',
      )
    })
  })
}
