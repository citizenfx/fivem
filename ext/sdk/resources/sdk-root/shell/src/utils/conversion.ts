export function toNumber(n: string | number): number {
  if (typeof n === 'string') {
    return parseInt(n, 10);
  }

  return n;
}
