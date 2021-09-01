export class Deferred<T = undefined> {
  resolve: {
    (): void;
    (v: T): void;
  };
  reject: (err?: any) => void;

  promise: Promise<T>;

  constructor() {
    this.promise = new Promise<T>((resolve, reject) => {
      this.resolve = resolve as any;
      this.reject = reject;
    });
  }
}
