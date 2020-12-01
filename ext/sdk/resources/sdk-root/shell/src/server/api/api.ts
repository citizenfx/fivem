import * as expressWs from "express-ws";
import * as ws from 'ws';
import { ApiClient, ApiEventCallback, ApiEventCallbackDisposer } from "shared/api.types";
import { AssetApi } from "./AssetApi";
import { ExplorerApi } from "./ExplorerApi";
import { ProjectApi } from "./ProjectApi";
import { ServerApi } from "./ServerApi";
import { ServerManagerApi } from "./ServerManagerApi";
import { StateApi } from "./StateApi";
import { StatusesApi } from "./StatusesApi";
import { NotificationsApi } from "./NotificationsApi";
import { FeaturesApi } from "./FeaturesApi";


const clients = new Set<ws>();

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

    listeners.add(cb);

    return () => {
      messageTypeListeners[eventType].delete(cb);
    };
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

    ws.on('message', async (msg: string) => {
      let type, data;

      try {
        [type, data] = JSON.parse(msg);
      } catch (e) {
        return console.log('Invalid api message:', msg, e);
      }

      const typedListeners = messageTypeListeners[type];
      if (!typedListeners?.size) {
        return console.log('No listeners for event of type:', type);
      }

      typedListeners.forEach(async (listener) => {
        try {
          await listener(data);
        } catch (e) {
          console.log('Unexpected error occured:', e);
          notifications.error(`Unexpected error occured: ${e.toString()}`);
        }
      });
    });
  });
};

export const notifications = new NotificationsApi(apiClient);
export const statuses = new StatusesApi(apiClient);

export const features = new FeaturesApi(apiClient, statuses);

export const serverManager = new ServerManagerApi(apiClient, notifications);
export const state = new StateApi(apiClient);
export const server = new ServerApi(apiClient, serverManager, features);
export const explorer = new ExplorerApi(apiClient, notifications);
export const project = new ProjectApi(apiClient, explorer, notifications);
export const asset = new AssetApi(apiClient, project);
