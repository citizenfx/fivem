export type WindowEventListener<T> = (data: T) => void;
export type WindowMessageListener = (data: any) => void;

export type WindowEventType = string;

const messageListeners: Set<WindowMessageListener> = new Set();
const eventListeners: Record<WindowEventType, Set<WindowEventListener<unknown>>> = Object.create(null);
const loadingEventListeners: Record<string, Set<WindowEventListener<unknown>>> = Object.create(null);

export const onWindowEvent = <DataType>(eventType: string, listener: WindowEventListener<DataType>): (() => void) => {
  if (!eventListeners[eventType]) {
    eventListeners[eventType] = new Set();
  }

  eventListeners[eventType].add(listener);

  return () => eventListeners[eventType].delete(listener);
};

export const onWindowMessage = (listener: WindowMessageListener): (() => void) => {
  messageListeners.add(listener);

  return () => messageListeners.delete(listener);
};

export const onLoadingEvent = <DataType>(eventName: string, listener: WindowEventListener<DataType>): (() => void) => {
  if (!loadingEventListeners[eventName]) {
    loadingEventListeners[eventName] = new Set();
  }

  loadingEventListeners[eventName].add(listener);

  return () => loadingEventListeners[eventName].delete(listener);
};

window.addEventListener('message', (event: MessageEvent) => {
  for (const listener of messageListeners) {
    listener(event.data);
  }

  if (typeof event.data !== 'object' || event.data === null) {
    return;
  }

  // Loadingscreen events
  if (event.data.eventName) {
    const listeners = loadingEventListeners[event.data.eventName];
    if (listeners) {
      for (const listener of listeners) {
        listener(event.data);
      }
    }

    return;
  }

  const { type, data } = event.data;
  if (!type) {
    return;
  }

  const listeners = eventListeners[type];
  if (!listeners || listeners.size === 0) {
    return;
  }

  for (const listener of listeners) {
    listener(data);
  }
});
