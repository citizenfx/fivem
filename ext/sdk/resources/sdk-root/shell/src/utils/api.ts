import ReconnectingWebScoket from 'reconnectingwebsocket';
import { enableLogger, errorLog, logger, rootLogger } from './logger';

const apiRxLog = logger('api:rx');
const apiTxLog = logger('api:tx');
const hostLog = rootLogger('host');

enableLogger('host');

export type ApiMessageListener = (data: any, type: string | typeof ANY_MESSAGE) => void;

const messageListeners: {
  [eventType: string]: Set<ApiMessageListener>,
} = {};

let pendingMessages: string[] = [];
let connected = false;

const ws = new ReconnectingWebScoket('ws://localhost:35419/api');

export const ANY_MESSAGE = Symbol('ANY_MESSAGE');

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

    apiRxLog(type, data);

    const typeListeners = new Set(messageListeners[type]);
    const anyListeners = new Set(messageListeners[ANY_MESSAGE as any]);

    if (typeListeners) {
      typeListeners.forEach((listener) => listener(data, type));
    }

    if (anyListeners) {
      anyListeners.forEach((listener) => listener(data, type));
    }
  } catch (e) {
    errorLog(e);
  }
});

export interface ApiMessage {
  type: string,
  data?: any,
};

export const sendApiMessage = (type: string, data?: any) => {
  apiTxLog(type, data);

  const message= JSON.stringify([type, data]);

  if (connected) {
    ws.send(message);
  } else {
    pendingMessages.push(message);
  }
}

export const onApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener) => {
  const listeners = messageListeners[type as any] || (messageListeners[type as any] = new Set());

  listeners.add(cb);

  return () => offApiMessage(type, cb);
};

export const offApiMessage = (type: string | typeof ANY_MESSAGE, cb: ApiMessageListener) => {
  const listeners = messageListeners[type as any];

  if (listeners) {
    listeners.delete(cb);
  }
}
