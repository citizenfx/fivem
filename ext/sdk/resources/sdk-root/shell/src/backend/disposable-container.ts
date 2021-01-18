export interface DisposableObject {
  dispose?(): void | Promise<void>;
}

export type Disposable =
  | DisposableObject
  | (() => any);

export class DisposableContainer {
  protected readonly container: Disposable[] = [];

  add(...disposables: Disposable[]) {
    this.container.push(...disposables);
  }

  async dispose(): Promise<void> {
    await Promise.all(this.container.map((disposable) => {
      if (typeof disposable === 'function') {
        return disposable();
      }

      return disposable.dispose?.();
    }));
  }
}
