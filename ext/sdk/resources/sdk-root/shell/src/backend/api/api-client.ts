import * as ws from 'ws';
import { inject, injectable, postConstruct } from 'inversify';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { LogService } from 'backend/logger/log-service';
import { ShellBackend } from 'backend/shell-backend';

export type ApiEventListener = <T extends any>(data: T) => void | Promise<void>;
export type ApiEventListenerDisposer = () => void;

@injectable()
export class ApiClient {
  public readonly onEventListenerError = new SingleEventEmitter<Error>();

  protected clients: Set<ws> = new Set();

  protected eventTypeListeners: Record<string, Set<ApiEventListener>> = {};
  protected eventTypeCallbackListener: Record<string, ApiEventListener> = {};

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  @inject(LogService)
  protected readonly logService: LogService;

  @postConstruct()
  protected initialize() {
    // on('sdk:api:recv', (msg: string) => {
    //   let type, data;

    //   try {
    //     [type, data] = JSON.parse(msg);
    //   } catch (e) {
    //     return this.logService.log('Invalid api message:', msg, e);
    //   }

    //   const typedListeners = this.eventTypeListeners[type];
    //   if (!typedListeners?.size) {
    //     return console.log('No listeners for event of type:', type);
    //   }

    //   typedListeners.forEach(async (listener) => {
    //     try {
    //       await listener(data);
    //     } catch (e) {
    //       this.onEventListenerError.emit(e);
    //       this.logService.log('Unexpected error occured:', e);
    //     }
    //   });
    // });

    this.shellBackend.expressApp.ws('/api', (ws) => {
      this.clients.add(ws);

      ws.on('close', () => this.clients.delete(ws));
      ws.on('message', (message: string) => this.handleMessage(message));
    });
  }

  emit<T extends any>(eventType: string, data?: T) {
    const message = JSON.stringify(
      typeof data !== 'undefined'
        ? [eventType, data]
        : [eventType],
    );

    // emit('sdk:api:send', message);
    this.clients.forEach((client) => client.send(message));
  }

  on(eventType: string, listener: ApiEventListener): ApiEventListenerDisposer {
    const listeners = this.eventTypeListeners[eventType] ??= new Set();

    listeners.add(listener);

    return () => listeners.delete(listener);
  }

  onCallback(eventType: string, listener: ApiEventListener): ApiEventListenerDisposer {
    if (this.eventTypeCallbackListener[eventType]) {
      throw new Error(`Unable to register new listener for callback event ${eventType} as it is already registered`);
    }

    this.eventTypeCallbackListener[eventType] = listener;

    return () => delete this.eventTypeCallbackListener[eventType];
  }

  off(eventType: string, listener: ApiEventListener) {
    this.eventTypeListeners[eventType]?.delete(listener);
  }

  offCallback(eventType: string) {
    delete this.eventTypeCallbackListener[eventType];
  }

  private async handleMessage(message: string) {
    let args: [string, any] | ['@@cb', string, string, any];

    try {
      args = JSON.parse(message);
    } catch (e) {
      this.logService.log('Invalid api message:', message, e);
      return;
    }

    if (args[0] === '@@cb') {
      const [, id, type, data] = args;

      const listener = this.eventTypeCallbackListener[type];
      if (!listener) {
        this.logService.log(`No listener for callback event ${type}`);
        return;
      }

      try {
        this.emitCallbackResponse(id, await listener(data));
      } catch (e) {
        this.emitCallbackError(id, e);
      }
    } else {
      const [type, data] = args;

      const typedListeners = this.eventTypeListeners[type];
      if (!typedListeners?.size) {
        return console.log('No listeners for event of type:', type);
      }

      typedListeners.forEach(async (listener) => {
        try {
          await listener(data);
        } catch (e) {
          this.onEventListenerError.emit(e);
          this.logService.log('Unexpected error occured:', e);
        }
      });
    }
  }

  private emitCallbackError(id: string, error: Error) {
    const message = JSON.stringify(
      ['@@cb', [ id, 'error', error + ''] ],
    );

    // emit('sdk:api:send', message);
    this.clients.forEach((client) => client.send(message));
  }

  private emitCallbackResponse(id: string, response: any) {
    const message = JSON.stringify(
      ['@@cb', [ id, 'response', response] ],
    );

    // emit('sdk:api:send', message);
    this.clients.forEach((client) => client.send(message));
  }
}
