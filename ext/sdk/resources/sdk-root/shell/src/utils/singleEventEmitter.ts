export type SingleEventListener<DataType extends any> = (data: DataType) => void;
export type SingleEventListenerDisposer = () => void;

export class SingleEventEmitter<DataType extends any> {
  protected listeners: Set<SingleEventListener<DataType>> = new Set();

  addListener(listener: SingleEventListener<DataType>): SingleEventListenerDisposer {
    this.listeners.add(listener);

    return () => this.listeners.delete(listener);
  }

  removeListener(listener: SingleEventListener<DataType>) {
    this.listeners.delete(listener);
  }

  emit(data: DataType) {
    this.listeners.forEach((listener) => listener(data));
  }
}

export type AsyncSingleEventListener<DataType extends any> = (data: DataType) => Promise<void> | void;

export class AsyncSingleEventEmitter<DataType extends any> {
  protected listeners: Set<SingleEventListener<DataType>> = new Set();

  addListener(listener: SingleEventListener<DataType>): SingleEventListenerDisposer {
    this.listeners.add(listener);

    return () => this.listeners.delete(listener);
  }

  removeListener(listener: SingleEventListener<DataType>) {
    this.listeners.delete(listener);
  }

  async emit(data: DataType) {
    await Promise.all([...this.listeners].map((listener) => listener(data)));
  }
}
