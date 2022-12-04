export function clamp01(n: number): number {
  if (n < 0) {
    return 0;
  }

  if (n > 1) {
    return 1;
  }

  return n;
}

export function clamp(n: number, min: number, max: number): number {
  return Math.min(max, Math.max(min, n));
}
