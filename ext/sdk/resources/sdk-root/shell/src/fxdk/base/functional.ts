export function noop(): void {

}

export function returnTrue(): true {
  return true;
}

export function returnFalse(): false {
  return false;
}

export function identity<T>(x: T): T {
  return x;
}
