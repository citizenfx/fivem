import { makeAutoObservable } from "mobx";

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
