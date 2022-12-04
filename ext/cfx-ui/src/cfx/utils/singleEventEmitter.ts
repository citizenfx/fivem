export type SingleEventListener<DataType extends any> = (data: DataType) => void;
export type SingleEventListenerDisposer = () => void;

export class SingleEventEmitter<DataType extends any> {
  protected listeners: Set<SingleEventListener<DataType>> = new Set();

  readonly addListener = (listener: SingleEventListener<DataType>): SingleEventListenerDisposer => {
    this.listeners.add(listener);

    return () => this.listeners.delete(listener);
  };

  readonly removeListener = (listener: SingleEventListener<DataType>) => {
    this.listeners.delete(listener);
  };

  readonly emit = (data: DataType) => {
    this.listeners.forEach((listener) => listener(data));
  };
}

export type AsyncSingleEventListener<DataType extends any> = (data: DataType) => Promise<void> | void;

export class AsyncSingleEventEmitter<DataType extends any> {
  protected listeners: Set<SingleEventListener<DataType>> = new Set();

  readonly addListener = (listener: SingleEventListener<DataType>): SingleEventListenerDisposer => {
    this.listeners.add(listener);

    return () => this.listeners.delete(listener);
  };

  readonly removeListener = (listener: SingleEventListener<DataType>) => {
    this.listeners.delete(listener);
  };

  readonly emit = async (data: DataType) => {
    await Promise.all([...this.listeners].map((listener) => listener(data)));
  };
}
