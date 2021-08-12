import { action, computed, makeAutoObservable } from "mobx";

function readLS<ValueType>(key: string, defaultValue: ValueType): ValueType {
  const value = localStorage[key];

  if (value === undefined) {
    return defaultValue;
  }

  try {
    return JSON.parse(localStorage[key]);
  } catch (e) {
    return defaultValue;
  }
}

export class LocalStorageValue<ValueType> {
  private _value: ValueType;

  constructor(
    private key: string,
    defaultValue: ValueType,
  ) {
    this._value = readLS(key, defaultValue);

    makeAutoObservable(this, {
      set: action,
      get: computed,
    });
  }

  set(value: ValueType) {
    this._value = value;

    localStorage[this.key] = JSON.stringify(value);
  }

  get(): ValueType {
    return this._value;
  }
}
