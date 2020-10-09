export function invariant(condition: unknown, errorMessage: string): asserts condition {
  if (!condition) {
    throw new Error(errorMessage);
  }
}

export function proxyInvariant<T>(condition: T | null | void | false, errorMessage: string | Function): T {
  if (!condition) {
    if (typeof errorMessage === 'function') {
      throw new Error(errorMessage());
    }

    throw new Error(errorMessage);
  }

  return condition;
}
