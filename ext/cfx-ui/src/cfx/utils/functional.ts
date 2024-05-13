// eslint-disable-next-line @typescript-eslint/no-empty-function
export function noop(): void {}

export function returnTrue(): true {
  return true;
}

export function returnFalse(): false {
  return false;
}

export function identity<T>(x: T): T {
  return x;
}

export function invoke<T extends () => void>(x: T): void {
  x();
}
