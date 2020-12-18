import { produce } from 'immer';

export class ChangeAwareContainer<T extends (object|Array<any>)> {
  constructor(
    protected snapshot: T,
    protected onApply: (snapshot: T) => void = () => {},
  ) { }

  get(): T {
    return this.snapshot;
  }

  apply(fn: (draft: T) => void) {
    this.snapshot = produce(this.snapshot, fn);
    this.onApply(this.snapshot);
  }
}
