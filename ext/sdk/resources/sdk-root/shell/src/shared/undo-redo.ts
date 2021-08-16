export interface UndoRedoItem {
  op: string | symbol | number,
}

type Ptr = number;

export class UndoRedo<Item extends UndoRedoItem> {
  private ptr: Ptr = 0; // current empty cell
  private start: Ptr = 0; // start cell
  private end: Ptr = 0; // end empty cell

  private items = new Map<Ptr, Item>();

  private get length(): number {
    return this.end - this.start;
  }

  constructor(
    private maxLength = 50,
  ) { }

  push(item: Item) {
    if (this.length > this.maxLength) {
      this.start++;
    }

    this.items.set(this.ptr, item);

    this.ptr++;
    this.end = this.ptr;

    this.gc();
  }

  undo(): Item | null {
    if (!this.length || this.ptr === this.start) {
      return null;
    }

    this.ptr--;

    return this.items.get(this.ptr);
  }

  redo(): Item | null {
    if (!this.length || this.ptr === this.end) {
      return null;
    }

    const item = this.items.get(this.ptr);

    this.ptr++;

    return item;
  }

  private rIC: number | null = null;
  private gc() {
    if (this.rIC !== null) {
      cancelIdleCallback(this.rIC);
    }

    this.rIC = requestIdleCallback(() => {
      this.rIC = null;

      this.items.forEach((_item, idx) => {
        if (idx < this.start || idx >= this.end) {
          this.items.delete(idx);
        }
      });
    });
  }
}
