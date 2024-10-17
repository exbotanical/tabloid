import assert from 'node:assert'
// Implementation copied from Neatpad

enum Action {
  SENTINEL,
  INSERT,
  DELETE,
  REPLACE,
}

class EventStack {
  private eventCaptures: PieceDescriptorRange[] = []

  empty() {
    return this.eventCaptures.length === 0
  }

  last() {
    return this.eventCaptures[this.eventCaptures.length - 1]
  }

  pop() {
    return this.eventCaptures.pop()
  }

  push(evRange: PieceDescriptorRange) {
    this.eventCaptures.push(evRange)
  }

  clear() {
    this.eventCaptures = []
  }

  back(index: number) {
    const length = this.eventCaptures.length
    if (length > 0 && index < length) {
      return this.eventCaptures[length - index - 1]
    }
    throw Error(`invalid index ${index}`)
  }
}

class SeqBuffer {
  length = 0
  maxSize = 0
  id = 0
  constructor(public buffer: string) {}
}

let id = -2
class PieceDescriptor {
  readonly id = id++

  constructor(
    public offset = 0,
    public length = 0,
    public buffer = 0,
    public next?: PieceDescriptor,
    public prev?: PieceDescriptor,
  ) {}
}

class PieceDescriptorRange {
  boundary = true

  constructor(
    public seqLength = 0,
    public index = 0,
    public length = 0,
    public first?: PieceDescriptor,
    public last?: PieceDescriptor,
  ) {}

  append(pd: PieceDescriptor) {
    if (!this.first) {
      this.first = pd
    } else {
      assert(this.last != null)

      this.last.next = pd
      pd.prev = this.last
    }

    this.last = pd
    this.boundary = false
  }

  appendRange(range: PieceDescriptorRange) {
    if (!range.boundary) {
      if (this.boundary) {
        this.first = range.first
        this.last = range.last
        this.boundary = false
      } else {
        assert(range.first != null)
        assert(this.last != null)

        range.first.prev = this.last
        this.last.next = range.first
        this.last = range.last
      }
    }
  }

  prependRange(range: PieceDescriptorRange) {
    if (!range.boundary) {
      if (this.boundary) {
        this.first = range.first
        this.last = range.last
        this.boundary = false
      } else {
        assert(range.last != null)
        assert(this.first != null)

        range.last.next = this.first
        this.first.prev = range.last
        this.first = range.first
      }
    }
  }

  asBoundary(before?: PieceDescriptor, after?: PieceDescriptor) {
    this.first = before
    this.last = after
    this.boundary = true
  }
}

class PieceTable {
  private readonly undoStack: EventStack = new EventStack()
  private readonly redoStack: EventStack = new EventStack()
  private frag1: PieceDescriptor | undefined
  private frag2: PieceDescriptor | undefined
  private head: PieceDescriptor = new PieceDescriptor()
  private tail: PieceDescriptor = new PieceDescriptor()
  private seqLength = 0
  private addBufferId = -1
  private bufferList: SeqBuffer[] = []
  private lastAction = Action.SENTINEL
  private lastActionIndex = 0

  constructor() {
    this.head.next = this.tail
    this.tail.prev = this.head
  }

  init(piece: string) {
    const length = piece.length
    const addBuffer = this.allocAddBuffer(length)

    addBuffer.buffer += piece
    addBuffer.length = length

    const id = this.bufferList.length - 1

    const pd = new PieceDescriptor(0, length, id, this.tail, this.head)
    this.head.next = pd
    this.tail.prev = pd
    this.seqLength = length

    this.recordAction(Action.SENTINEL, 0)
  }

  size() {
    return this.seqLength
  }

  insert(index: number, piece: string) {
    const length = piece.length
    if (index > this.seqLength) {
      return
    }

    const [pd, pdIndex] = this.descFromIndex(index)
    const addBufOffset = this.importBuffer(piece, length)

    this.redoStack.clear()

    const insertOff = index - pdIndex
    const newPds = new PieceDescriptorRange()

    // Inserting at the end of a prior insertion - at a pd boundary
    if (insertOff === 0 && this.canOptimize(Action.INSERT, index)) {
      assert(pd.prev != null)
      // Extend the last pd's length
      const ev = this.undoStack.last()
      pd.prev.length += length
      ev.length += length

      // Inserting at a pd boundary
    } else if (insertOff === 0) {
      const oldPds = this.initUndoRange(index, length)

      oldPds.asBoundary(pd.prev, pd)

      newPds.append(new PieceDescriptor(addBufOffset, length, this.addBufferId))

      this.swapDescRanges(oldPds, newPds)
      // Inserting in the middle of a piece
    } else {
      const oldPds = this.initUndoRange(index, length)

      oldPds.append(pd)
      newPds.append(new PieceDescriptor(pd.offset, insertOff, pd.buffer))
      newPds.append(new PieceDescriptor(addBufOffset, length, this.addBufferId))
      newPds.append(
        new PieceDescriptor(
          pd.offset + insertOff,
          pd.length - insertOff,
          pd.buffer,
        ),
      )

      this.swapDescRanges(oldPds, newPds)
    }

    this.seqLength += length

    this.recordAction(Action.INSERT, index + length)
  }

  delete(index: number, length: number) {
    if (
      length === 0 ||
      length > this.seqLength ||
      index > this.seqLength - length
    ) {
      console.error('nope!')
      return
    }

    let [pd, pdIndex] = this.descFromIndex(index)

    const rmOffset = index - pdIndex

    let rmLength = length
    let appendPdRange = false

    let ev

    const newPds = new PieceDescriptorRange()
    const oldPds = new PieceDescriptorRange()

    // Forward-delete
    if (index === pdIndex && this.canOptimize(Action.DELETE, index)) {
      ev = this.undoStack.back(0)
      ev.length += length
      appendPdRange = true

      if (this.frag2) {
        if (length < this.frag2.length) {
          this.frag2.length -= length
          this.frag2.offset += length
          this.seqLength -= length
          return
        } else {
          rmLength -= pd.length
          pd = pd.next!
          this.deleteFromDesc(this.frag2)
        }
      }
      // Backward delete
    } else if (
      index + length === pdIndex + pd.length &&
      this.canOptimize(Action.DELETE, index + length)
    ) {
      ev = this.undoStack.last()
      ev.length += length
      ev.index -= index
      appendPdRange = false

      if (this.frag1) {
        if (length < this.frag1.length) {
          this.frag1.length -= length
          this.frag1.offset += 0
          this.seqLength -= length
          return
        } else {
          rmLength -= this.frag1.length
          this.deleteFromDesc(this.frag1)
        }
      }
    } else {
      appendPdRange = true
      this.frag1 = this.frag2 = undefined

      ev = this.initUndoRange(index, length)
    }

    this.redoStack.clear()

    // Deletion starts midway through a piece
    if (rmOffset !== 0) {
      newPds.append(new PieceDescriptor(pd.offset, rmOffset, pd.buffer))
      this.frag1 = newPds.first

      if (rmOffset + rmLength < pd.length) {
        newPds.append(
          new PieceDescriptor(
            pd.offset + rmOffset + rmLength,
            pd.length - rmOffset - rmLength,
            pd.buffer,
          ),
        )

        this.frag2 = newPds.last
      }

      rmLength -= Math.min(rmLength, pd.length - rmOffset)

      oldPds.append(pd)
      pd = pd.next!
    }

    while (rmLength > 0 && pd !== this.tail) {
      if (rmLength < pd.length) {
        newPds.append(
          new PieceDescriptor(
            pd.offset + rmLength,
            pd.length - rmLength,
            pd.buffer,
          ),
        )

        this.frag2 = newPds.last
      }

      rmLength -= Math.min(rmLength, pd.length)
      oldPds.append(pd)
      pd = pd.next!
    }

    this.swapDescRanges(oldPds, newPds)
    this.seqLength -= length

    if (appendPdRange) {
      ev.appendRange(oldPds)
    } else {
      ev.prependRange(oldPds)
    }

    this.recordAction(Action.DELETE, index)
  }

  undo() {
    return this.doStackEvent(this.undoStack, this.redoStack)
  }

  redo() {
    return this.doStackEvent(this.redoStack, this.undoStack)
  }

  deleteFromDesc(pd: PieceDescriptor) {
    assert(pd.prev != null)
    assert(pd.next != null)

    pd.prev.next = pd.next
    pd.next.prev = pd.prev
  }

  doStackEvent(src: EventStack, dest: EventStack) {
    if (src.empty()) return
    this.recordAction(Action.SENTINEL, 0)
    let range

    do {
      range = src.last()
      src.pop()
      dest.push(range)
      this.restoreDescRanges(range)
    } while (!src.empty())
  }

  private initUndoRange(index: number, length: number): PieceDescriptorRange {
    const ev = new PieceDescriptorRange(this.seqLength, index, length)

    this.undoStack.push(ev)
    return ev
  }

  private allocBuffer(maxSize: number): SeqBuffer {
    const buf = new SeqBuffer('')
    buf.length = 0
    buf.maxSize = maxSize
    buf.id = this.bufferList.length
    this.bufferList.push(buf)
    return buf
  }

  private allocAddBuffer(maxSize: number) {
    const buf = this.allocBuffer(maxSize)
    this.addBufferId = buf.id
    return buf
  }

  private importBuffer(s: string, length: number): number {
    let buf = this.bufferList[this.addBufferId]
    if (buf.length + length >= buf.maxSize) {
      buf = this.allocAddBuffer(length + 0x10000)
      // Ensure no old pds use this buffer
      this.recordAction(Action.SENTINEL, 0)
    }

    buf.buffer += s

    const ret = buf.length
    buf.length += length

    return ret
  }

  private swapDescRanges(
    src: PieceDescriptorRange,
    dest: PieceDescriptorRange,
  ) {
    assert(src.first != null)
    assert(src.last != null)

    if (src.boundary) {
      if (!dest.boundary) {
        assert(dest.first != null)
        assert(dest.last != null)

        src.first.next = dest.first
        src.last.prev = dest.last
        dest.first.prev = src.first
        dest.last.next = src.last
      }
    } else {
      assert(src.first.prev != null)
      assert(src.last.next != null)

      if (dest.boundary) {
        src.first.prev.next = src.last.next
        src.last.next.prev = src.first.prev
      } else {
        assert(dest.first != null)
        assert(dest.last != null)

        src.first.prev.next = dest.first
        src.last.next.prev = dest.last
        dest.first.prev = src.first.prev
        dest.last.next = src.last.next
      }
    }
  }

  private restoreDescRanges(range: PieceDescriptorRange) {
    if (range.boundary) {
      const first = range.first?.next
      const last = range.last?.prev

      assert(range.first != null)
      assert(range.last != null)

      // unlink descs from main list
      range.first.next = range.last
      range.last.prev = range.first

      // store the desc range we just removed
      range.first = first
      range.last = last
      range.boundary = false
    } else {
      let first = range.first?.prev
      let last = range.last?.next

      assert(first != null)
      assert(last != null)

      // are we moving descs into an "empty" region?
      // (i.e. in between two adjacent descs)
      if (first.next === last) {
        // move the old descs back into the empty region
        first.next = range.first
        last.prev = range.last

        // store the desc range we just removed
        range.first = first
        range.last = last
        range.boundary = true
      }
      // we are replacing a range of descs in the list -
      // swap the descs in the list with those in our undo ev
      else {
        // find the desc range that is currently in the list
        first = first.next
        last = last.prev

        assert(first?.prev != null)
        assert(last?.next != null)

        assert(last != null)

        // unlink the the descs from the main list
        first.prev.next = range.first
        last.next.prev = range.last

        // store the desc range we just removed
        range.first = first
        range.last = last
        range.boundary = false
      }
    }

    let tmp = range.seqLength
    range.seqLength = this.seqLength
    this.seqLength = tmp
  }

  private descFromIndex(index: number): [PieceDescriptor, number] {
    let pd: PieceDescriptor | undefined
    let currIndex = 0
    let pdIndex = 0

    for (pd = this.head.next; pd?.next; pd = pd.next) {
      if (index >= currIndex && index < currIndex + pd.length) {
        // if (pdIndex) {
        pdIndex = currIndex
        // }

        return [pd, pdIndex]
      }

      currIndex += pd.length
    }

    // Insert at tail
    if (pd && index === currIndex) {
      pdIndex = currIndex
      return [pd, pdIndex]
    }

    throw Error('descFromIndex')
  }

  recordAction(action: Action, index: number) {
    this.lastAction = action
    this.lastActionIndex = index
  }

  canOptimize(action: Action, index: number) {
    return this.lastAction === action && this.lastActionIndex === index
  }

  render(index = 0, length = this.seqLength) {
    // Cache
    let s = ''
    let total = 0

    let [pd, pdIndex] = this.descFromIndex(index)
    let pdOffset = index - pdIndex

    while (length && pd !== this.tail) {
      const l = Math.min(pd.length - pdOffset, length)
      const src = this.bufferList[pd.buffer].buffer
      const start = pd.offset + pdOffset
      const end = start + l
      s += src.substring(start, end)

      length -= l
      total += l
      pdOffset = 0
      pd = pd.next!
    }

    return s
  }
}

if (import.meta.vitest) {
  const { test } = import.meta.vitest

  test('piece table', () => {
    const pt = new PieceTable()
    pt.init('hello world')
    pt.insert(3, 'goodbye')
    // helgoodbyelo world
    pt.insert(6, 'xx')
    // helgooxxdbyelo world
    pt.delete(3, 9)
    // hello world
    pt.delete(0, 6)
    // world
    pt.undo()
    // hello world
    pt.redo()
    // world
    pt.insert(5, '   xx')
    pt.insert(5, '   yy')

    // console.log({ s: pt.render() })
  })

  test('editor', () => {
    const gridSize = {
      cols: 100,
      rows: 10,
    }

    const editorGrid = ['hello', 'world', 'this', 'is a line']
    const raw = editorGrid.join('\n')
    //               1     1
    //        6      2     7
    // 'hello\nworld\nthis\nis a line'

    const pt = new PieceTable()
    pt.init(raw)

    let lineStarts: number[] = []

    let lineBuffer = ''
    function initLineBuffer() {
      lineBuffer = ''
      const docSize = pt.size()

      let offsetChars = 0
      let lineStart = 0
      let numLines = 0
      lineStarts = []

      for (; offsetChars < docSize; ) {
        const char = pt.render(offsetChars++, 1)
        lineBuffer += char

        if (char === '\n') {
          lineStarts[numLines] = lineStart
          lineStart = offsetChars
          numLines++
        }
      }

      lineStarts[numLines++] = lineStart
      lineStarts[numLines] = offsetChars
    }

    function paint() {
      for (let row = 0; row < gridSize.rows; row++) {
        const line = getline(row)

        if (!line) continue
        for (let col = 0; col < gridSize.cols; col++) {
          const char = line[col]
          if (char) {
            process.stdout.write(char)
          } else break
        }
      }
    }

    function paintDivider() {
      process.stdout.write('\n')
      process.stdout.write('\n')
      process.stdout.write('---')
      process.stdout.write('\n')
      process.stdout.write('\n')
    }

    function getAbsoluteChar({ x, y }: { x: number; y: number }) {
      return lineStarts[y] + x
    }

    function insert(cursor: { x: number; y: number }, char: string) {
      pt.insert(getAbsoluteChar(cursor), char)
      initLineBuffer()
    }

    function remove(cursor: { x: number; y: number }) {
      pt.delete(getAbsoluteChar(cursor), 1)
      initLineBuffer()
    }

    function getline(lineno: number) {
      // We don't actually process the last line start - it's just there so we can grab an offset with the lineno+1 lookahead
      if (lineStarts.length <= lineno + 1) {
        return null
      }

      const lineLength = lineStarts[lineno + 1] - lineStarts[lineno]

      return lineBuffer.substring(
        lineStarts[lineno],
        lineStarts[lineno] + lineLength,
      )
    }

    initLineBuffer()
    paint()
    paintDivider()

    // Let's make some edits...
    insert({ x: 5, y: 0 }, 'x')

    paint()
    paintDivider()

    insert({ x: 1, y: 1 }, 'x')
    insert({ x: 3, y: 1 }, 'x')
    insert({ x: 3, y: 3 }, 'x')
    insert({ x: 5, y: 3 }, 'x')

    paint()
    paintDivider()

    remove({ x: 5, y: 0 })
    remove({ x: 2, y: 1 })
    remove({ x: 4, y: 3 })
    insert({ x: 3, y: 3 }, 'x')

    paint()
    paintDivider()
  })
}

// TODO: cleanup

// lineStarts: [ 0, 6, 12, 17, 26 ],
// full: 'hello\nworld\nthis\nis a line'

// lineStarts: [ 0, 7, 13, 18, 27 ],
// full: 'hellox\nworld\nthis\nis a line'
