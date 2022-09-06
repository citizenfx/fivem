import { NotFunction } from "./types";

export type ValueOrGetter<T extends NotFunction> = T | (() => T)

export function getValue<T extends NotFunction>(value: ValueOrGetter<T>): T {
  if (typeof value === 'function') {
    return value();
  }

  return value;
}
