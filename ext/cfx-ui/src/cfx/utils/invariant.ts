type Falsy = false | 0 | '' | null | undefined;

export function invariant(cond: any, message: string): asserts cond {
  if (!cond) {
    throw new Error(message);
  }
}

export function proxyInvariant<T>(cond: T | Falsy, message: string): T {
  invariant(cond, message);

  return cond as any;
}
