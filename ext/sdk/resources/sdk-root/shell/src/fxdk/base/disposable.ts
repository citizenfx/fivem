export interface IDisposableObject {
  dispose?(): void | Promise<void>;
}

export type IDisposable =
  | IDisposableObject
  | (() => any);

export class Disposer {
  protected readonly container: IDisposable[] = [];

  add(...disposables: IDisposable[]) {
    this.container.push(...disposables);
  }

  register<T extends IDisposable>(disposable: T): T {
    this.add(disposable);

    return disposable;
  }

  async dispose(): Promise<void> {
    await Promise.all(this.container.map((disposable) => {
      if (typeof disposable === 'function') {
        return disposable();
      }

      return disposable.dispose?.();
    }));
  }

  empty(): boolean {
    return this.container.length === 0;
  }
}

export function disposableFromFunction(dispose: () => void): IDisposableObject {
  return { dispose };
}

export function dispose(disposable: IDisposable | undefined) {
  if (!disposable) {
    return;
  }

  if (typeof disposable === 'function') {
    return disposable();
  }

  return disposable.dispose?.();
}
