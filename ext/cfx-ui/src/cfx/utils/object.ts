export function isObject<T extends object | undefined>(obj: unknown): obj is NonNullable<T> {
  return typeof obj === 'object' && !Array.isArray(obj) && obj !== null;
}

export function clone<T extends object | any[]>(obj: T): T {
  return JSON.parse(JSON.stringify(obj));
}
