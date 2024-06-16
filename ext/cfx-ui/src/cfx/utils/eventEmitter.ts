import { IDisposable } from './disposable';
import { SingleEventListener } from './singleEventEmitter';

export class MultiEventEmitter<TPayload> {
  private listeners: Record<string, Set<SingleEventListener<TPayload>>> = {};

  readonly addListener = (type: string, cb: SingleEventListener<TPayload>): IDisposable => {
    if (!this.listeners[type]) {
      this.listeners[type] = new Set();
    }

    this.listeners[type].add(cb);

    return () => this.listeners[type].delete(cb);
  };

  readonly removeListener = (type: string, cb: SingleEventListener<TPayload>) => {
    this.listeners[type]?.delete(cb);
  };

  readonly emit = (type: string, data: TPayload) => {
    if (this.listeners[type]) {
      this.listeners[type].forEach((cb) => {
        cb(data);
      });
    }
  };
}
