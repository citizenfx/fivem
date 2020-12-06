import * as ws from 'ws';
import { inject, injectable, postConstruct } from 'inversify';
import { SingleEventEmitter } from 'server/single-event-emitter';
import { CommandService } from 'server/commands/command-service';
import { LogService } from 'server/logger/log-service';
import { ShellBackend } from 'server/shell-backend';

export type ApiEventListener = <T extends any>(data: T) => void;
export type ApiEventListenerDisposer = () => void;

@injectable()
export class ApiClient {
  public readonly onEventListenerError = new SingleEventEmitter<Error>();

  protected clients: Set<ws> = new Set();

  protected eventTypeListeners: Record<string, Set<ApiEventListener>> = {};

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(CommandService)
  protected readonly commandService: CommandService;

  @postConstruct()
  protected initialize() {
    this.shellBackend.expressApp.ws('/api', (ws) => {
      this.clients.add(ws);

      ws.on('close', () => {
        this.clients.delete(ws);
      });

      ws.on('message', async (msg: string) => {
        let type, data;

        try {
          [type, data] = JSON.parse(msg);
        } catch (e) {
          return this.logService.log('Invalid api message:', msg, e);
        }

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
      });
    });
  }

  emit<T extends any>(eventType: string, data?: T) {
    const message = JSON.stringify(
      typeof data !== 'undefined'
        ? [eventType, data]
        : [eventType],
    );

    this.clients.forEach((client) => client.send(message));
  }

  on(eventType: string, listener: ApiEventListener): ApiEventListenerDisposer {
    const listeners = this.eventTypeListeners[eventType] ??= new Set();

    listeners.add(listener);

    return () => listeners.delete(listener);
  }

  off(eventType: string, listener: ApiEventListener) {
    this.eventTypeListeners[eventType]?.delete(listener);
  }
}
