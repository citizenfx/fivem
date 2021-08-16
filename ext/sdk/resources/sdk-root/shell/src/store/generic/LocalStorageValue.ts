import { makeAutoObservable } from "mobx";

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

export interface LocalStorageValueConfig<ValueType> {
  key: string,
  defaultValue: ValueType,

  readTransform?(value: ValueType): ValueType,
}

export class LocalStorageValue<ValueType> {
  private key: string;
  private _value: ValueType;

  constructor(
    config: LocalStorageValueConfig<ValueType>,
  ) {
    this.key = config.key;

    this._value = readLS(this.key, config.defaultValue);

    if (config.readTransform) {
      this._value = config.readTransform(this._value);
    }

    makeAutoObservable(this);
  }

  set(value: ValueType) {
    this._value = value;

    localStorage[this.key] = JSON.stringify(value);
  }

  get(): ValueType {
    return this._value;
  }
}
