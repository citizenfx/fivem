let endpointIndex = 0;

function nextEndpointIndex(): string {
  return (endpointIndex++).toString(16);
}

export function apiEndpointSingle(ep: string): string {
  return nextEndpointIndex();
}

export function apiEndpointsGroup<T extends { [key: string ]: string }>(eps: T): { [Key in keyof T]: string } {
  return Object.keys(eps).reduce((acc, key) => {
    acc[key] = nextEndpointIndex();

    return acc;
  }, {} as any);
}
