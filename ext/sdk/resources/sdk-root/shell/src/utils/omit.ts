export function omit<T extends object>(obj: T, key: string): T {
  const newObj = { ...obj };

  delete newObj[key];

  return newObj;
}
