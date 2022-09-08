import { action, makeAutoObservable, makeObservable, _autoAction } from "mobx";
import { Deferred } from "./async";

export interface IObservableValue<T> {
  get(): T;
}

export class ObservableAsyncValue<T> implements IObservableValue<T> {
  static from<Value>(fetcherFn: () => Promise<Value>): ObservableAsyncValue<Value> {
    return new ObservableAsyncValue(fetcherFn);
  }

  private _state = ObservableRemoteValueState.Fetching;

  private _value: T;
  private _error: any;
  private _deferred?: Deferred<void>;

  private constructor(fetcherFn) {
    makeObservable(this, {
      // @ts-expect-error privates
      _state: true,

      set: action,
      error: action,

      get: _autoAction,
      hasValue: _autoAction,

      getError: _autoAction,
      hasError: _autoAction,
    });

    this.fetch(fetcherFn);
  }

  private async fetch(fetcherFn) {
    try {
      this.set(await fetcherFn());
    } catch (e) {
      this.error(e);
    }
  }

  private set(value: T) {
    if (this._state !== ObservableRemoteValueState.Fetching) {
      throw new Error('Impossible set after reject or resolve');
    }

    this._state = ObservableRemoteValueState.Resolved;
    this._value = value;
    this._deferred?.resolve();
  }

  private error(error: any) {
    if (this._state !== ObservableRemoteValueState.Fetching) {
      throw new Error('Impossible error after reject or resolve');
    }

    this._state = ObservableRemoteValueState.Rejected;
    this._error = error;
    this._deferred?.reject(error);
  }

  public hasError(): boolean {
    return this._state === ObservableRemoteValueState.Rejected;
  }

  public hasValue(): boolean {
    return this._state === ObservableRemoteValueState.Resolved;
  }

  public getError(): any {
    return this._error;
  }

  public get(): Readonly<T> {
    if (this._state !== ObservableRemoteValueState.Resolved) {
      throw new Error('Value is not loaded yet or there was an error loading it');
    }

    return this._value;
  }

  public async wait(): Promise<void> {
    if (!this._deferred) {
      this._deferred = new Deferred();
    }

    await this._deferred.promise;
  }

  public async waitGet(): Promise<Readonly<T>> {
    if (!this._deferred) {
      this._deferred = new Deferred();
    }

    await this._deferred.promise;

    return this.get();
  }

  public map<ToValue>(mapFn: (value: T) => ToValue): ObservableAsyncValue<ToValue> {
    return ObservableAsyncValue.from(async () => {
      await this.wait();

      return mapFn(this.get());
    });
  }
}
const enum ObservableRemoteValueState {
  Fetching,
  Resolved,
  Rejected,
}

export class AwaitableValue<T> {
  private _deferred = new Deferred<void>();
  private _resolved = false;

  get value(): T {
    return this._value;
  }

  set value(newValue: T) {
    this._value = newValue;

    if (!this._resolved) {
      this._resolved = true;

      this._deferred.resolve();
    }
  }

  constructor(private _value: T) {
    makeAutoObservable(this);
  }

  resolved(): Promise<void> {
    return this._deferred.promise;
  }

  isResolved(): boolean {
    return this._resolved;
  }
}
