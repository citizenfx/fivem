export class Deferred<T> {
  resolve: (value?: T) => void;
  reject: (err?: any) => void;

  promise = new Promise<T>((resolve, reject) => {
      this.resolve = resolve;
      this.reject = reject;
  });
}
