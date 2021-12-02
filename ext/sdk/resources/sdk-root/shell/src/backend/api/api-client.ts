import * as ws from 'ws';
import { inject, injectable, postConstruct } from 'inversify';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { ShellBackend } from 'backend/shell-backend';
import { ScopedLogService } from 'backend/logger/scoped-logger';
import { Ticker } from 'backend/ticker';

const wsSendOptions = {
  binary: true,
};

export type ApiEventListener = <T extends any>(data: T) => void | Promise<void>;
export type ApiEventListenerDisposer = () => void;

@injectable()
export class ApiClient {
  public readonly onEventListenerError = new SingleEventEmitter<Error>();
  public readonly onClientConnected = new SingleEventEmitter<void>();

  protected clients: Set<ws.WebSocket> = new Set();

  protected eventTypeListeners: Record<string, Set<ApiEventListener>> = {};
  protected eventTypeCallbackListener: Record<string, ApiEventListener> = {};

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  protected readonly logService = new ScopedLogService('ApiClient');

  @postConstruct()
  protected initialize() {
    this.shellBackend.useWS('/api', (ws) => {
      this.clients.add(ws);

      ws.on('close', () => this.clients.delete(ws));
      ws.on('message', (message: string) => this.handleMessage(message));

      this.onClientConnected.emit();
    });
  }

  emit<T extends any>(eventType: string, data?: T) {
    this.broadcastMessage(
      typeof data !== 'undefined'
        ? [eventType, data]
        : [eventType],
    );
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
      args = JSON.parse(message.toString());
    } catch (e) {
      this.logService.log('Invalid api message:', message, e);
      return;
    }

    if (args[0] === '@@cb') {
      const [, id, type, data] = args;
      if (!type) {
        throw new Error(`No type specified in message ${message}`);
      }

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
    this.broadcastMessage(['@@cb', [id, 'error', error + '']]);
  }

  private emitCallbackResponse(id: string, response: any) {
    this.broadcastMessage(['@@cb', [id, 'response', response]]);
  }

  private messagesQueue: [string, any?][] = [];
  private messagesQueueTicker = new Ticker();

  private broadcastMessage(message: [string, any?]) {
    this.messagesQueue.push(message);

    this.messagesQueueTicker.whenTickEnds(this.workMessageQueue);
  }

  private readonly workMessageQueue = () => {
    const mq = this.messagesQueue;
    this.messagesQueue = [];

    let ms: string;

    if (mq.length === 1) {
      ms = JSON.stringify(mq[0]);
    } else {
      ms = JSON.stringify(['##mq', mq]);
    }

    const mb = Buffer.from(ms);

    for (const client of this.clients) {
      client.send(mb, wsSendOptions);
    }
  };
}
