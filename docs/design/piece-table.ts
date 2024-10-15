import assert from 'node:assert'
// Implementation copied from Neatpad

enum Action {
  SENTINEL,
  INSERT,
  DELETE,
  REPLACE,
}

class EventStack {
  private evs: PieceDescriptorRange[] = []

  constructor() {}
  empty() {
    return this.evs.length === 0
  }

  last() {
    return this.evs[this.evs.length - 1]
  }

  pop() {
    return this.evs.pop()
  }

  push(evRange: PieceDescriptorRange) {
    this.evs.push(evRange)
  }

  clear() {
    this.evs = []
  }
}

class SeqBuffer {
  buffer: string
  length: number = 0
  maxSize: number = 0
  id: number = 0
  constructor(buffer: string) {
    this.buffer = buffer
  }
}

let id = -2
class PieceDescriptor {
  next?: PieceDescriptor
  prev?: PieceDescriptor
  id = id++
  length: number
  constructor(
    readonly offset = 0,
    length = 0,
    readonly buffer = 0,
    next?: PieceDescriptor,
    prev?: PieceDescriptor,
  ) {
    this.length = length
    // TODO: cleanup
    if (next) {
      this.next = next
    }
    if (prev) {
      this.prev = prev
    }
  }
}

class PieceDescriptorRange {
  first?: PieceDescriptor
  last?: PieceDescriptor
  boundary = true
  seqLength: number
  length: number

  constructor(
    seqLength = 0,
    readonly index = 0,
    length = 0,
    first?: PieceDescriptor,
    last?: PieceDescriptor,
  ) {
    this.seqLength = seqLength
    this.length = length
    this.first = first
    this.last = last
  }

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
    if (range.boundary == false) {
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
  private head: PieceDescriptor = new PieceDescriptor()
  private tail: PieceDescriptor = new PieceDescriptor()

  private frag1: PieceDescriptor | undefined
  private frag2: PieceDescriptor | undefined

  private undoStack: EventStack = new EventStack()
  private redoStack: EventStack = new EventStack()

  private seqLength = 0
  private addBufferId = -1
  private bufferList: SeqBuffer[] = []

  lastAction = Action.SENTINEL
  lastActionIndex = 0
  constructor() {
    this.head.next = this.tail
    this.tail.prev = this.head
  }

  init(piece: string) {
    const length = piece.length
    const b = this.allocAddBuffer(length)
    b.buffer += piece
    b.length = length
    const id = this.bufferList.length - 1

    const pd = new PieceDescriptor(0, length, id, this.tail, this.head)
    this.head.next = pd
    this.tail.prev = pd
    this.seqLength = length

    this.recordAction(Action.SENTINEL, 0)
  }

  insert(index: number, piece: string) {
    const length = piece.length
    if (index > this.seqLength) {
      return
    }

    const r = this.descFromIndex(index)
    if (!r) return

    const [pd, pdIndex] = r
    const addBufOffset = this.importBuffer(piece, length)

    this.redoStack.clear()

    const insertOff = index - pdIndex
    const newPds = new PieceDescriptorRange()
    let oldPds = new PieceDescriptorRange()

    // Inserting at the end of a preceding insertion - at a pd boundary
    if (insertOff === 0 && this.canOptimize(Action.INSERT, index)) {
      assert(pd.prev != null)
      // Extend the last pd's length
      const ev = this.undoStack.last()
      pd.prev.length += length
      ev.length += length

      // Inserting at a pd boundary
    } else if (insertOff === 0) {
      oldPds = this.initUndoRange(index, length)

      oldPds.asBoundary(pd.prev, pd)
      newPds.append(new PieceDescriptor(addBufOffset, length, this.addBufferId))

      this.swapDescRanges(oldPds, newPds)
    } else {
      oldPds = this.initUndoRange(index, length)

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
    const appendPdRange = true

    const ev = this.initUndoRange(index, length)

    const newPds = new PieceDescriptorRange()
    const oldPds = new PieceDescriptorRange()

    if (index === pdIndex && this.canOptimize(Action.DELETE, index)) {
    } else if (
      index + length === pdIndex + pd.length &&
      this.canOptimize(Action.DELETE, index + length)
    ) {
    } else {
    }

    this.redoStack.clear()

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

    let tmp = range.seqLen
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

  debug() {
    let length = this.seqLength

    let s = ''
    const index = 0
    let total = 0

    let [pd, pdIndex] = this.descFromIndex(index)
    let pdOffset = index - pdIndex

    while (length && pd !== this.tail) {
      const l = Math.min(pd.length - pdOffset, length)
      const src = this.bufferList[pd.buffer].buffer
      console.log({ HERE: pd.id })
      const start = pd.offset + pdOffset
      const end = start + l
      s += src.substring(start, end)

      length -= l
      total += l
      pdOffset = 0
      pd = pd.next!
    }
    console.log({ s })
  }
}

if (import.meta.vitest) {
  const { it } = import.meta.vitest

  it('test', () => {
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

    pt.debug()
  })
}

// TODO: cleanup
