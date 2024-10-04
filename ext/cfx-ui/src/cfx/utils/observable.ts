import { action, makeAutoObservable, makeObservable, _autoAction } from 'mobx';

import { Deferred } from './async';

export interface IObservableValue<T> {
  get(): T;
}

export interface IObservableAsyncValueOptions {
  lazy?: boolean;
}

const enum ObservableRemoteValueState {
  Idle = 1,
  Fetching = 2,
  Resolved = 3,
  Rejected = 4,
}

export class ObservableAsyncValue<T> implements IObservableValue<T> {
  static from<Value>(
    fetcherFn: () => Promise<Value>,
    options?: IObservableAsyncValueOptions,
  ): ObservableAsyncValue<Value> {
    return new ObservableAsyncValue(fetcherFn, options);
  }

  private _state: ObservableRemoteValueState = ObservableRemoteValueState.Idle;

  private _value: T;

  private _error: any;

  private _deferred?: Deferred<void>;

  private constructor(
    private readonly fetcherFn: () => Promise<T>,
    private readonly options: IObservableAsyncValueOptions = {},
  ) {
    makeObservable(this, {
      // @ts-expect-error privates
      _state: true,

      fetcherFn: false,
      options: false,

      set: action,
      error: action,

      get: _autoAction,
      hasValue: _autoAction,

      getError: _autoAction,
      hasError: _autoAction,
    });

    if (!options.lazy) {
      this.ensureFetchRequested();
    }
  }

  private async ensureFetchRequested() {
    if (this._state >= ObservableRemoteValueState.Fetching) {
      return;
    }

    this._state = ObservableRemoteValueState.Fetching;

    try {
      this.set(await this.fetcherFn());
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

    this.ensureFetchRequested();

    await this._deferred.promise;
  }

  public async waitGet(): Promise<Readonly<T>> {
    await this.wait();

    return this.get();
  }

  public map<ToValue>(mapFn: (value: T) => ToValue): ObservableAsyncValue<ToValue> {
    return ObservableAsyncValue.from(async () => {
      await this.wait();

      return mapFn(this.get());
    }, this.options);
  }
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
