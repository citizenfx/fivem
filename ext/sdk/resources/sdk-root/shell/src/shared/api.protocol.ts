export function apiEndpointSingle(ep: string): string {
  return ep;
}

export function apiEndpointsGroup<T extends { [key: string ]: string }>(eps: T): { [Key in keyof T]: string } {
  return eps;
}

export function defineApiEndpoints<T extends string>(ns: string, endpoints: T[]): { [key in T]: string } {
  return endpoints.reduce((collection, ep) => {
    collection[ep] = `${ns}.${ep}`;

    return collection;
  }, {} as any);
}
