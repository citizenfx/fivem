import { SystemEvent } from "../systemEvents";

const $$clientEvents = Symbol('clientEvents');
const $$systemEvents = Symbol('systemEvents');

export interface ClientEventBinding {
  propKey: string,
  eventName: string,
}

export interface SystemEventBinding {
  propKey: string,
  event: SystemEvent,
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


export function handlesSystemEvent(event: SystemEvent) {
  return function (target: any, propKey: string) {
    target[$$systemEvents] = target[$$systemEvents] || [];
    target[$$systemEvents].push({ propKey, event });
  }
}

export function getSystemEventHandlers(target: any): SystemEventBinding[] {
  return target[$$systemEvents] || [];
}
