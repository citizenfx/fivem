export function arrayAt<T>(array: T[], index: number): T | undefined {
  if (index < 0) {
    return array[array.length + index];
  }

  return array[index];
}
