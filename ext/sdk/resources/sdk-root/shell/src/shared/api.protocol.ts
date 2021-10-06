export function apiEndpointSingle(ep: string): string {
  return ep;
}

export function apiEndpointsGroup<T extends { [key: string ]: string }>(eps: T): { [Key in keyof T]: string } {
  return eps;
}
