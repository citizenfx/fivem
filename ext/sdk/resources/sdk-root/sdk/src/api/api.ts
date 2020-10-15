import * as expressWs from "express-ws";
import * as ws from 'ws';
import { ApiClient, ApiEventCallback, ApiEventCallbackDisposer } from "./api.types";
import { AssetApi } from "./AssetApi";
import { ExplorerApi } from "./ExplorerApi";
import { ProjectApi } from "./ProjectApi";
import { ServerApi } from "./ServerApi";
import { ServerManagerApi } from "./ServerManagerApi";
import { StateApi } from "./StateApi";


const clients = new Set<ws>();

const messageListeners: Set<ApiEventCallback<any>> = new Set();
const messageTypeListeners: { [eventType: string]: Set<ApiEventCallback<any>> } = {};


export const apiClient: ApiClient = {
  emit<T>(eventType: string, data?: T): void {
    const message = JSON.stringify([eventType, data]);

    for (const client of clients) {
      client.send(message);
    }
  },
  emitSid<T>(sid: string, eventType: string, data?: T): void {
    return this.emit(`${eventType}(${sid})`, data);
  },

  on<T>(eventType: string, cb: ApiEventCallback<T>): ApiEventCallbackDisposer {
    const listeners = messageTypeListeners[eventType] || (messageTypeListeners[eventType] = new Set());

    this.log('listening to event', eventType);

    listeners.add(cb);

    return () => {
      this.log('stopped listening to event', eventType);
      messageTypeListeners[eventType].delete(cb);
    };
  },
  onAny<T>(cb: ApiEventCallback<T>): ApiEventCallbackDisposer {
    messageListeners.add(cb);

    return () => messageListeners.delete(cb);
  },

  log(...args: any[]): void {
    return this.emit('@@log', args);
  },
};

export const mountApi = (app: expressWs.Application) => {
  app.ws('/api', (ws) => {
    clients.add(ws);

    ws.on('close', () => {
      clients.delete(ws);
    });

    ws.on('message', (msg: string) => {
      try {
        const [type, data] = JSON.parse(msg);

        messageListeners.forEach((listener) => listener(type, data));

        const typedListeners = messageTypeListeners[type];

        apiClient.log('num of listeners of message type', type, 'is', typedListeners?.size);

        if (typedListeners) {
          typedListeners.forEach((listener) => listener(data));
        }
      } catch (e) {
        console.error(e);
      }
    });
  });
};

export const serverManager = new ServerManagerApi(apiClient);
export const state = new StateApi(apiClient);
export const server = new ServerApi(apiClient, serverManager);
export const explorer = new ExplorerApi(apiClient);
export const project = new ProjectApi(apiClient, explorer);
export const asset = new AssetApi(apiClient, project);
