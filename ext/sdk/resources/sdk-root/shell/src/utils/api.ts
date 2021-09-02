import { Deferred } from 'backend/deferred';
import ReconnectingWebScoket from 'reconnectingwebsocket';
import { getScopedEventName } from './apiScope';
import { enableLogger, errorLog, logger, rootLogger } from './logger';
import { fastRandomId } from './random';

const apiRxLog = logger('api:rx');
const apiTxLog = logger('api:tx');
const hostLog = rootLogger('host');

enableLogger('host');

export const ANY_MESSAGE = Symbol('ANY_MESSAGE');
export type ApiMessageListener = (data: any, type: string | typeof ANY_MESSAGE) => void;
export interface ApiMessageCallback<Data> {
  (error: string, data: void): void;
  (error: null, data: Data): void;
}

const messageListeners: {
  [eventType: string]: Set<ApiMessageListener>,
} = {};
const callbackDeferreds: Record<string, Deferred<unknown>> = {};
const messageCallbacks: Record<string, ApiMessageCallback<any>> = {};

let pendingMessages: string[] = [];
let connected = false;
const backlog: { [type: string]: any[] } = {};

const ws = new ReconnectingWebScoket('ws://localhost:35419/api');
ws.addEventListener('open', () => {
  connected = true;

  if (pendingMessages.length > 0) {
    pendingMessages.forEach((message) => ws.send(message));
    pendingMessages = [];
  }
});
ws.addEventListener('close', () => {
  connected = false;
});
ws.addEventListener('message', (event: MessageEvent) => {
  try {
    const [type, data] = JSON.parse(event.data);

    if (type === '@@log') {
      const [msg, ...args] = data;

      return hostLog(msg, ...args);
    }

    if (type === '@@cb') {
      const [id, type, res] = data;

      const callback = messageCallbacks[id];
      if (!callback) {
        apiRxLog('No callback for message', id);
        return;
      }

      delete messageCallbacks[id];

      if (type === 'error') {
        return callback(res, undefined);
      } else {
        return callback(null, res);
      }
    }

    // apiRxLog(type, data);

    const typeListeners = new Set(messageListeners[type]);
    const anyListeners = new Set(messageListeners[ANY_MESSAGE as any]);

    if (typeListeners) {
      typeListeners.forEach((listener) => listener(data, type));
    }

    if (typeListeners?.size === 0) {
      if (!backlog[type]) {
        backlog[type] = [];
      }

      backlog[type].push(data);
    }

    if (anyListeners) {
      anyListeners.forEach((listener) => listener(data, type));
    }
  } catch (e) {
    errorLog(e);
  }
});

// window.addEventListener('message', (event) => {
//   try {
//     if (event.data.type !== 'sdkApiMessage') {
//       return;
//     }

//     const [type, data] = event.data.data;

//     if (type === '@@log') {
//       const [msg, ...args] = data;

//       return hostLog(msg, ...args);
//     }

//     apiRxLog(type, data);

//     const typeListeners = new Set(messageListeners[type]);
//     const anyListeners = new Set(messageListeners[ANY_MESSAGE as any]);

//     if (typeListeners) {
//       typeListeners.forEach((listener) => listener(data, type));
//     }

//     if (anyListeners) {
//       anyListeners.forEach((listener) => listener(data, type));
//     }
//   } catch (e) {
//     errorLog(e);
//   }
// });

export interface ApiMessage {
  type: string,
  data?: any,
};

export const sendApiMessage = (type: string, data?: any) => {
  // apiTxLog(type, data);

  const message = JSON.stringify([type, data]);

  // fxdkSendApiMessage(message);

  if (connected) {
    ws.send(message);
  } else {
    pendingMessages.push(message);
  }
}
export const sendApiMessageScoped = (type: string, scope: string, data?: any) => {
  sendApiMessage(getScopedEventName(type, scope), data);
};

export const sendApiMessageCallback = <ResponseData>(type: string, data: any, callback: ApiMessageCallback<ResponseData>): (() => void) => {
  const id = fastRandomId();

  messageCallbacks[id] = callback;

  const message = JSON.stringify(['@@cb', id, type, data]);
  console.log(message);

  if (connected) {
    ws.send(message);
  } else {
    pendingMessages.push(message);
  }

  return () => delete messageCallbacks[id];
};
export const sendApiMessageCallbackScoped = <ResponseData>(type: string, scope: string, data: any, callback: ApiMessageCallback<ResponseData>): (() => void) => {
  return sendApiMessageCallback(getScopedEventName(type, scope), data, callback);
};

export const onApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener) => {
  const listeners = messageListeners[type as any] || (messageListeners[type as any] = new Set());

  listeners.add(cb);

  if (type !== ANY_MESSAGE && backlog[type]) {
    for (const entry of backlog[type]) {
      cb(entry, type);
    }

    delete backlog[type];
  }

  return () => offApiMessage(type, cb);
};
export const onApiMessageScoped = (type: string, scope: string, cb: ApiMessageListener) => {
  return onApiMessage(getScopedEventName(type, scope), cb);
};

export const offApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener) => {
  const listeners = messageListeners[type as any];

  if (listeners) {
    listeners.delete(cb);
  }
}
export const offApiMessageScoped = (type: string, scope: string, cb: ApiMessageListener) => {
  return offApiMessage(getScopedEventName(type, scope), cb);
};
