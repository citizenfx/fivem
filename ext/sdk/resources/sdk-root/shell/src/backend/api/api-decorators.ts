const $$clientEvents = Symbol('clientEvents');
const $$clientCallbackEvents = Symbol('clientCallbackEvents');

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

export function handlesClientCallbackEvent(eventName: string) {
  return function (target: any, propKey: string) {
    target[$$clientCallbackEvents] = target[$$clientCallbackEvents] || [];
    target[$$clientCallbackEvents].push({ propKey, eventName });
  }
}

export function getClientEventHandlers(target: any): ClientEventBinding[] {
  return target[$$clientEvents] || [];
}

export function getClientCallbackEventHandlers(target: any): ClientEventBinding[] {
  return target[$$clientCallbackEvents] || [];
}
