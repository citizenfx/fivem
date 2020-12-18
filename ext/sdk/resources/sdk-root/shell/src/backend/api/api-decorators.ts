const $$clientEvents = Symbol('clientEvents');

export interface ClientEventBinding {
  propKey: string,
  eventName: string,
}

export function handlesClientEvent(eventName: string) {
  return function (target: any, propKey: string) {
    target[$$clientEvents] = target[$$clientEvents] || [];
    target[$$clientEvents].push({ propKey, eventName });
  }
}

export function getClientEventHandlers(target: any): ClientEventBinding[] {
  return target[$$clientEvents] || [];
}
