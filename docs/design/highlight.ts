function highlight() {
  const HIGHLIGHT_CHAR = 'H'
  const cursor = { x: 0, y: 0 }
  const hl = {
    active: false,
    off: -1,
    startRow: -1,
    endRow: -1,
  }
  const numCols = 10

  function setCursor(x: number, y: number) {
    cursor.x = x
    cursor.y = y
  }

  function moveLeft() {
    if (cursor.x == 0) {
      if (cursor.y === 0) return
      setCursor(numCols - 1, cursor.y - 1)
    } else {
      cursor.x--
    }
  }

  function moveRight() {
    if (cursor.x == numCols - 1) {
      setCursor(0, cursor.y + 1)
    } else {
      cursor.x++
    }
  }

  let highlightStack = 0

  function highlightRight() {
    if (!hl.active) {
      hl.off = cursor.x
      hl.startRow = cursor.y
    }
    hl.active = true
    moveRight()
    hl.endRow = cursor.y

    highlightStack++
  }

  function highlightLeft() {
    if (!hl.active) {
      hl.off = cursor.x
      hl.startRow = cursor.y
    }
    hl.active = true
    moveLeft()
    hl.endRow = cursor.y

    highlightStack--
  }

  function highlightClear() {
    Object.assign(highlight, {
      startRow: -1,
      endRow: -1,
      off: -1,
      active: false,
    })
  }

  // simulation
  setCursor(8, 0)
  highlightRight()
  highlightRight()
  // highlightRight()
  // highlightRight()
  // highlightRight()
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()
  // // - back on first line
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()
  // highlightLeft()

  // highlightRight()
  // highlightRight()
  // highlightRight()
  // highlightRight()
  // highlightRight()
  // highlightRight()
  // // - back to second line
  // highlightRight()
  // highlightRight()
  // highlightRight()

  console.log({ hl })

  const grid = { rows: 10, cols: numCols }
  const editorState = ['fishtanksh', 'world', 'what']

  let isCursorEnd, start, end

  if (hl.startRow === hl.endRow) {
    isCursorEnd = cursor.x > hl.off
    start = isCursorEnd ? hl.off : cursor.x
    end = isCursorEnd ? cursor.x : hl.off
  } else {
    isCursorEnd = true
    start = hl.off
    end = cursor.x
  }

  console.log({ cursor, start, end })

  for (let i = 0; i < grid.rows; i++) {
    for (let j = 0; j < grid.cols; j++) {
      const maybeChar = editorState[i]?.[j]
      if (maybeChar) {
        if (hl.active) {
          if (i < hl.startRow) {
            process.stdout.write(maybeChar)
            continue
          }

          if (i > hl.endRow) {
            process.stdout.write(maybeChar)
            continue
          }

          if (i === hl.startRow && i === hl.endRow) {
            if (j >= start && j <= end) {
              process.stdout.write(HIGHLIGHT_CHAR)
            } else {
              process.stdout.write(maybeChar)
            }
            continue
          }

          if (i === hl.startRow) {
            if (j >= start) {
              process.stdout.write(HIGHLIGHT_CHAR)
            } else {
              process.stdout.write(maybeChar)
            }
            continue
          }

          if (i === hl.endRow) {
            if (j <= end) {
              process.stdout.write(HIGHLIGHT_CHAR)
            } else {
              process.stdout.write(maybeChar)
            }
            continue
          }

          if (i > hl.startRow && i < hl.endRow) {
            process.stdout.write(HIGHLIGHT_CHAR)
            continue
          }

          process.stdout.write(maybeChar)
        } else {
          process.stdout.write(maybeChar)
        }
      } else {
        process.stdout.write('.')
      }
    }
    process.stdout.write('\n')
  }
  process.stdout.write(JSON.stringify({ highlight }))
}

if (import.meta.vitest) {
  const { test, expect } = import.meta.vitest

  test('test', () => {
    highlight()
  })
}
