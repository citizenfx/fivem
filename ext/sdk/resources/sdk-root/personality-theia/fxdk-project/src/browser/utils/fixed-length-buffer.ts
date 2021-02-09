import { Disposable } from "@theia/core";

export type FixedLiengthBufferRemoveListener<ItemType> = (item: ItemType) => void;

export class FixedLengthBuffer<ItemType> implements Disposable {
  protected nextKey = 0;
  protected firstKey = 0;
  protected currentLength = 0;
  protected buffer: Record<number, ItemType> = Object.create(null);
  protected removeListeners: Set<FixedLiengthBufferRemoveListener<ItemType>> = new Set();

  constructor(
    protected readonly maxLength,
  ) { }

  get length(): number {
    return this.currentLength;
  }

  push(...items: ItemType[]) {
    for (const item of items) {
      this.pushOne(item);
    }
  }

  onRemove(listener: FixedLiengthBufferRemoveListener<ItemType>) {
    this.removeListeners.add(listener);
  }

  dispose() {
    this.buffer = {};
    this.removeListeners = new Set();
  }

  protected pushOne(item: ItemType) {
    if (this.currentLength >= this.maxLength) {
      const keyToDrop = this.firstKey++;
      const itemToDrop = this.buffer[keyToDrop];

      delete this.buffer[keyToDrop];

      this.removeListeners.forEach((listener) => listener(itemToDrop));
    } else {
      this.currentLength++;
    }

    const key = this.nextKey++;

    this.buffer[key] = item;
  }
}
