import { runInAction } from 'mobx';
import ReconnectingWebScoket from 'reconnectingwebsocket';
import { OpenFlag } from 'store/generic/OpenFlag';
import { apiHost } from 'utils/apiHost';
import { getScopedEventName } from 'utils/apiScope';
import { enableLogger, errorLog, logger, rootLogger } from 'utils/logger';
import { noop } from 'utils/noop';
import { fastRandomId } from 'utils/random';
import { ShellLifecycle, ShellLifecyclePhase } from './shellLifecycle';


const apiRxLog = logger('api:rx');
const apiTxLog = logger('api:tx');
const hostLog = rootLogger('host');

enableLogger('host');

export type ApiMessageListener = (data: any, type: string) => void;
export interface ApiMessageCallback<Data> {
  (error: string, data: void): void;
  (error: null, data: Data): void;
}

export const Api = new class Api {
  public readonly connection = new OpenFlag();

  private ws: ReconnectingWebScoket | null = null;

  private pendingPackets: string[] = [];
  private incomingStash: Record<string, unknown[]> = {};

  private listeners: Record<string, Set<ApiMessageListener>> = {};
  private callbacks: Record<string, ApiMessageCallback<unknown>> = {};

  constructor() {
    ShellLifecycle.onPhase(ShellLifecyclePhase.Started, () => {
      this.initialize();
    });
  }

  private initialized = false;
  initialize() {
    if (this.initialized) {
      throw new Error('Api already initialized');
    }

    this.initialized = true;

    this.ws = new ReconnectingWebScoket(`ws://${apiHost.host}:${apiHost.port}/api`);

    this.ws.addEventListener('open', () => {
      console.log('Hello connection');
      this.connection.open();

      if (this.pendingPackets.length) {
        for (const packet of this.pendingPackets) {
          this.sendPacket(packet);
        }

        this.pendingPackets = [];
      }
    });

    this.ws.addEventListener('close', () => {
      this.connection.close();
    });

    this.ws.addEventListener('message', this.messageHandler);
  }

  on(type: string, cb: ApiMessageListener): () => void {
    const listeners = (this.listeners[type] ??= new Set());
    listeners.add(cb);

    if (this.incomingStash[type]) {
      const dataItems = this.incomingStash[type];
      delete this.incomingStash[type];

      for (const data of dataItems) {
        cb(data, type);
      }
    }

    return () => listeners.delete(cb);
  }
  onScoped(type: string, scope: string, cb: ApiMessageListener): () => void {
    return this.on(getScopedEventName(type, scope), cb);
  }
  onWithAction(type: string, cb: ApiMessageListener): () => void {
    const _cb = (data) => runInAction(() => cb(data, type));

    return this.on(type, _cb);
  }
  onScopeWithAction(type: string, scope: string, cb: ApiMessageListener): () => void {
    return this.onWithAction(getScopedEventName(type, scope), cb);
  }

  send(type: string, data?: unknown) {
    this.sendPacket(JSON.stringify([type, data]));
  }
  sendScoped(type: string, scope: string, data?: unknown) {
    this.send(getScopedEventName(type, scope), data);
  }

  sendCallback<Response>(type: string, data: unknown, cb: ApiMessageCallback<Response>): () => void {
    const id = fastRandomId();

    this.callbacks[id] = cb;

    this.sendPacket(JSON.stringify(['@@cb', id, type, data]));

    return () => {
      this.callbacks[id] = noop;
    };
  }
  sendCallbackScoped<Response>(type: string, scope: string, data: unknown, cb: ApiMessageCallback<Response>): () => void {
    return this.sendCallback(getScopedEventName(type, scope), data, cb);
  }
  sendPromise<Response>(type: string, data?: unknown): Promise<Response> {
    return new Promise((resolve, reject) => {
      this.sendCallback(type, data, (error, response) => {
        if (error) {
          return reject(error);
        }

        return resolve(response);
      });
    });
  }
  sendPromiseScoped<Response>(type: string, scope: string, data?: unknown): Promise<Response> {
    return new Promise((resolve, reject) => {
      this.sendCallbackScoped(type, scope, data, (error, response) => {
        if (error) {
          return reject(error);
        }

        return resolve(response);
      });
    });
  }

  private messageHandler = async (event: MessageEvent<Blob>) => {
    try {
      const [type, data] = JSON.parse(await event.data.text());

      // apiRxLog(type, data);

      if (type === '##mq') {
        for (const [stype, sdata] of data) {
          this.handleSingleMessage(stype, sdata);
        }
      } else {
        this.handleSingleMessage(type, data);
      }
    } catch (e) {
      errorLog(e);
    }
  };

  private handleSingleMessage(type: string, data: any) {
    switch (type) {
      case '@@log': return hostLog(...(data as [string, ...any]));

      case '@@cb': {
        const [id, type, res] = data;

        const cb = this.callbacks[id];
        if (!cb) {
          apiRxLog('No callback for', id);
          return;
        }

        delete this.callbacks[id];

        if (type === 'error') {
          return cb(res, undefined);
        }

        return cb(null, res);
      }

      default: {
        const listeners = this.listeners[type];
        if (!listeners || listeners.size === 0) {
          this.incomingStash[type] ??= [];
          this.incomingStash[type].push(data);
          return;
        }

        for (const listener of listeners) {
          listener(data, type);
        }
      }
    }
  }

  private sendPacket(packet: string) {
    if (!this.ws || !this.connection.isOpen) {
      console.log('Adding packet to pending', packet);
      this.pendingPackets.push(packet);

      return;
    }

    this.ws.send(Buffer.from(packet));
  }
}();
