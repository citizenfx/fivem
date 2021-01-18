export function getResourceEventName(resourceName: string, event: string): string {
  return `resource:${resourceName}:${event}`;
}
