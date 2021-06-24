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
