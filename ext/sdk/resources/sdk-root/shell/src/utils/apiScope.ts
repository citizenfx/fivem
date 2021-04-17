export function getScopedEventName(eventName: string, scope: string): string {
  return `${scope}(${eventName})`;
}
