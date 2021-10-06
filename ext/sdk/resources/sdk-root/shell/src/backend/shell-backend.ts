import * as ws from 'ws';
import http from 'http';
import cors from 'cors';
import express from 'express';
import { Container, inject, injectable } from 'inversify';
import { ConfigService } from './config-service';
import { SingleEventEmitter } from '../utils/singleEventEmitter';
import { LogService } from './logger/log-service';
import { AppContribution, bindAppContribution } from './app/app-contribution';
import { Deferred } from './deferred';
import { URL } from 'url';

type Socket = import('net').Socket;
type UpgradeHandler = (request: http.IncomingMessage, socket: Socket, head: Buffer) => void | Promise<void>;

type WSHandler = (ws: ws.WebSocket, request: http.IncomingMessage) => void | Promise<void>;

@injectable()
export class ShellBackend implements AppContribution {
  public readonly onStarted = new SingleEventEmitter<void>();

  public readonly expressApp: express.Application;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(LogService)
  protected readonly logService: LogService;

  private server: http.Server;
  private wsServer: ws.WebSocketServer;

  private upgradeListeners: Record<string, UpgradeHandler> = {};

  constructor() {
    this.expressApp = (express() as any);

    this.server = http.createServer(this.expressApp);
    this.wsServer = new ws.WebSocketServer({ noServer: true });

    this.expressApp.disable('x-powered-by');

    // Applying CORS
    this.expressApp.use(cors({ origin: '*' }));

    // Handling WebSocket connections
    this.server.on('upgrade', (req: http.IncomingMessage, socket: Socket, head: Buffer) => {
      if (req.headers['upgrade'] !== 'websocket') {
        return socket.destroy(new Error('Unknown upgrade header'));
      }

      if (!req.url) {
        return socket.destroy(new Error('No url in request'));
      }

      try {
        const url = new URL(req.url, 'http://dummy.host');

        const handler = this.upgradeListeners[url.pathname];
        if (!handler) {
          return socket.destroy(new Error(`No handler for pathname: ${url.pathname}`));
        }

        handler(req, socket, head);
      } catch (e) {
        console.error(`Failed to handle upgrade for url: ${req.url}`, e);
      }
    });
  }

  boot() {
    const expressAppBootComplete = new Deferred();

    // Setting up static folder to serve
    this.expressApp.use(express.static(this.configService.sdkRootShellBuild));

    this.server.listen(this.configService.shellBackendPort, () => {
      this.logService.log(`Shell backend is listening on http://localhost:${this.configService.shellBackendPort}`);

      expressAppBootComplete.resolve();
    });

    return expressAppBootComplete.promise;
  }

  useStatic(at: string, rootPath: string) {
    this.expressApp.use(at, express.static(rootPath));
  }

  useUpgrade(pathname: string, handler: UpgradeHandler) {
    this.upgradeListeners[pathname] = handler;
  }

  useWS(pathname: string, handler: WSHandler) {
    if (this.upgradeListeners[pathname]) {
      throw new Error(`Unable to WS handler for ${pathname} as it is already registered`);
    }

    this.upgradeListeners[pathname] = (request, socket, head) => {
      this.wsServer.handleUpgrade(request, socket, head, handler);
    };
  }
}

export const bindShellBackend = (container: Container) => {
  container.bind(ShellBackend).toSelf().inSingletonScope();
  bindAppContribution(container, ShellBackend);
};
