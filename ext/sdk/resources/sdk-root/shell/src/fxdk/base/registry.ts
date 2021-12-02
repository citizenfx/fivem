import { makeAutoObservable, observable } from "mobx";

export interface IBaseRegistryItem {
  readonly id: string;
}

export class Registry<Item extends IBaseRegistryItem> {
  private items: Item[] = [];
  private itemIds = new Set<string>();

  constructor(
    private registryName: string,
    observable = false,
  ) {
    if (observable) {
      makeAutoObservable(this);
    }
  }

  register(item: Item) {
    if (this.has(item.id)) {
      throw new Error(`Cannot register ${this.registryName} item as it is already registered`);
    }

    this.items.push(item);
    this.itemIds.add(item.id);
  }

  has(id: string): boolean {
    return this.itemIds.has(id);
  }

  getAll() {
    return this.items;
  }
}

export class AssocRegistry<Item, Key = string> {
  private items = new Map<Key, Item>();

  constructor(
    private registryName: string,
    _observable = false,
  ) {
    if (_observable) {
      makeAutoObservable(this, {
        // @ts-expect-error it's fine though
        items: observable.shallow,
      });
    }
  }

  register(key: Key, item: Item) {
    this.items.set(key, item);
  }

  has(key: Key): boolean {
    return this.items.has(key);
  }

  get(key: Key): Item | undefined {
    return this.items.get(key);
  }

  getAll() {
    return this.items.values();
  }

  unregister(key: Key) {
    this.items.delete(key);
  }

  reset() {
    this.items.clear();
  }
}
