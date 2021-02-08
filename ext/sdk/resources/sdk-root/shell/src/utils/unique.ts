export function uniqueArray<T>(arr: T[]): T[] {
  return [...(new Set(arr))];
}
