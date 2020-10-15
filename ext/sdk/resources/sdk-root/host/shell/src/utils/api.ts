import ReconnectingWebScoket from 'reconnectingwebsocket';


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
      return console.log('[sdk]', ...(data || []));
    }

    console.log('[api] received message, type:', type, 'data:', data);

    const typeListeners = new Set(messageListeners[type]);
    const anyListeners = new Set(messageListeners[ANY_MESSAGE as any]);

    if (typeListeners) {
      typeListeners.forEach((listener) => listener(data, type));
    }

    if (anyListeners) {
      anyListeners.forEach((listener) => listener(data, type));
    }
  } catch (e) {
    console.error(e);
  }
});

export interface ApiMessage {
  type: string,
  data?: any,
};

export const sendApiMessage = (type: string, data?: any) => {
  console.log('[api] sending message, type:', type, 'data:', data);

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
