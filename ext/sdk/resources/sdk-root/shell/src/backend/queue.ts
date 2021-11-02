import { IDisposableObject } from "../fxdk/base/disposable";
import { Ticker } from "./ticker";

export const MAX_QUEUE_SIZE = 65536;

export class Queue<ItemType> implements IDisposableObject {
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

  appendMany(items: ItemType[]) {
    if (this.disposed) {
      return;
    }

    if (items.length === 0) {
      return;
    }

    this.queueSize += items.length;

    for (const item of items) {
      this.queue[this.getNextIndex()] = item;
    }

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

export class UniqueQueue<ItemType> implements IDisposableObject {
  private disposed = false;

  protected items: Record<string, ItemType> = Object.create(null);
  protected queue: Record<number, string> = Object.create(null);
  protected queueSize = 0;

  protected firstIndex = 0;
  protected nextIndex = 0;

  protected processing = false;

  protected ticker = new Ticker();

  constructor(
    private processItem: (item: ItemType) => Promise<void> | void,
    private hashItem: (item: ItemType) => string,
  ) {

  }

  append(item: ItemType) {
    if (this.disposed) {
      return;
    }

    if (this.doAppend(item) && !this.processing) {
      this.ticker.whenTickEnds(() => this.process());
    }
  }

  appendMany(items: ItemType[]) {
    if (this.disposed) {
      return;
    }

    let shouldProcess = false;

    for (const item of items) {
      const itemShouldUpdate = this.doAppend(item);
      shouldProcess ||= itemShouldUpdate;
    }

    if (!this.processing && shouldProcess) {
      this.ticker.whenTickEnds(() => this.process());
    }
  }

  protected doAppend(item: ItemType): boolean {
    const itemHash = this.hashItem(item);
    if (this.items[itemHash]) {
      return false;
    }

    this.items[itemHash] = item;

    this.queueSize++;
    this.queue[this.getNextIndex()] = itemHash;

    return true;
  }

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

  dispose() {
    if (this.disposed) {
      return;
    }

    this.queue = {};
    this.items = {};
    this.queueSize = 0;
  }

  private async process() {
    this.processing = true;

    while(this.queueSize > 0) {
      this.queueSize--;

      const index = this.getFirstIndex();
      const itemHash = this.queue[index];
      const item = this.items[itemHash];

      delete this.queue[index];
      delete this.items[itemHash];

      await this.processItem(item);
    }

    this.processing = false;
  }
}

function copy(obj: any): any {
  return JSON.parse(JSON.stringify(obj));
}
