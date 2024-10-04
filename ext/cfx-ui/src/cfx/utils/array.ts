export function arrayAt<T>(array: T[], index: number): T | undefined {
  if (index < 0) {
    return array[array.length + index];
  }

  return array[index];
}

export function uniqueArray<T>(array: Array<T>): Array<T> {
  return [...new Set(array)];
}

export function reverseArray<T>(array: Array<T>): Array<T> {
  return array.slice().reverse();
}

export function arrayAll<T>(array: Array<T>, predicate: (item: T) => boolean): boolean {
  return array.reduce((acc, item) => acc && predicate(item), true);
}

export function arraySome<T>(array: Array<T>, predicate: (item: T) => boolean): boolean {
  return array.some(predicate);
}

export function randomArrayItem<T>(array: Array<T>): T {
  return array[Math.floor(Math.random() * array.length)];
}
