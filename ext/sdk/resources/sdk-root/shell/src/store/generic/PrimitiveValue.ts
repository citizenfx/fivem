import { computed, makeAutoObservable, observable } from "mobx";

export class PrimitiveValue<T> {
  get value(): T {
    return this._value;
  }

  constructor(private _value: T) {
    makeAutoObservable(this, {
      // @ts-expect-error yeah
      _value: observable.ref,
    });
  }

  set(value: T) {
    this._value = value;
  }
}
