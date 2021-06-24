import { DisposableObject } from "./disposable-container";

export const MAX_QUEUE_SIZE = 65536;

export class Queue<ItemType> implements DisposableObject {
  protected queue: Record<number, ItemType> = Object.create(null);
  protected queueSize = 0;

  protected firstIndex = 0;
  protected nextIndex = 0;

  protected processing = false;
  protected disposed = false;

  constructor(
    protected processItem: (item: ItemType) => Promise<void> | void,
  ) {}

  protected getNextIndex() {
    if (this.nextIndex === MAX_QUEUE_SIZE) {
      return this.nextIndex = 0;
    }

    const nextIndex = this.nextIndex++;

    if (this.queue[nextIndex]) {
      throw new Error('Queue overflow');
    }

    return nextIndex;
  }

  protected getFirstIndex() {
    if (this.firstIndex === MAX_QUEUE_SIZE) {
      return this.firstIndex = 0;
    }

    return this.firstIndex++;
  }

  append(item: ItemType) {
    if (this.disposed) {
      return;
    }

    this.queueSize++;
    this.queue[this.getNextIndex()] = item;

    if (!this.processing) {
      this.process();
    }
  }

  dispose() {
    this.disposed = true;
    this.queueSize = 0;
  }

  private async process() {
    this.processing = true;

    while(this.queueSize > 0) {
      this.queueSize--;

      const index = this.getFirstIndex();
      const item = this.queue[index];

      delete this.queue[index];

      await this.processItem(item);
    }

    this.processing = false;
  }
}
