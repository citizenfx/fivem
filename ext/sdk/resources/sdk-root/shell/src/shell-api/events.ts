import { AsyncSingleEventEmitter, AsyncSingleEventListener, SingleEventEmitter, SingleEventListener, SingleEventListenerDisposer } from "utils/singleEventEmitter";

export interface IShellEvents {
  on<T>(eventName: string, listener: AsyncSingleEventListener<T>): SingleEventListenerDisposer;
  emit<T>(eventName: string, data?: T): Promise<void>;
  emitByObject<T>({ type, data }: { type: string, data?: T }): Promise<void>;
}

export const ShellEvents = new class ShellEvents implements IShellEvents {
  #events: Record<string, AsyncSingleEventEmitter<any>> = {};

  on<T>(eventName: string, listener: SingleEventListener<T>): SingleEventListenerDisposer {
    this.#events[eventName] ??= new AsyncSingleEventEmitter<T>();

    return this.#events[eventName].addListener(listener);
  }

  async emit<T>(eventName: string, data?: T): Promise<void> {
    if (this.#events[eventName]) {
      await this.#events[eventName].emit(data);
    }
  }

  async emitByObject<T>({ type, data }: { type: string, data?: T }): Promise<void> {
    await this.emit(type, data);
  }
}();
