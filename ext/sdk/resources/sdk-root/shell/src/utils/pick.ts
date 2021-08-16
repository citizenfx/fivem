export function pick<TObject extends {}, TKeys extends keyof TObject>(obj: TObject, ...keys: TKeys[]): Pick<TObject, TKeys> {
  return keys.reduce((acc, key) => {
    acc[key as any] = obj[key];

    return acc;
  }, {}) as any;
}
