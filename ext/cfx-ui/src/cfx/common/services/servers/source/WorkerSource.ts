import { CurrentGameName } from 'cfx/base/gameRuntime';
import { MultiEventEmitter } from 'cfx/utils/eventEmitter';
import { isObject } from 'cfx/utils/object';
import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';

import type { ServersWorker, ServerResponsePayload } from './WorkerSource.worker';

import { IAutocompleteIndex, IServerListSource } from './types';
import { IServerListConfig } from '../lists/types';
import { IPinnedServersConfig, IServerView } from '../types';

export class WorkerSource implements IServerListSource {
  private worker: Worker;

  private serversFetchStartEvent = new SingleEventEmitter<void>();

  public readonly onServersFetchStart = this.serversFetchStartEvent.addListener;

  private serversFetchChunkEvent = new SingleEventEmitter<IServerView[]>();

  public onServersFetchChunk = this.serversFetchChunkEvent.addListener;

  private serversFetchEndEvent = new SingleEventEmitter<IServerView[]>();

  public onServersFetchEnd = this.serversFetchEndEvent.addListener;

  private serversFetchErrorEvent = new SingleEventEmitter<string>();

  public onServersFetchError = this.serversFetchErrorEvent.addListener;

  private listEvent = new MultiEventEmitter<string[]>();

  public onList = this.listEvent.addListener;

  private indexEvent = new SingleEventEmitter<IAutocompleteIndex>();

  public onIndex = this.indexEvent.addListener;

  constructor() {
    this.worker = new Worker(new URL('./WorkerSource.worker.ts', import.meta.url));
    this.worker.addEventListener('message', this.handleWorkerMessage);
  }

  init() {
    console.log('INITING SERVERS LIST FOR GAME', CurrentGameName);

    this.sendToWorker('init', {
      gameName: CurrentGameName,
      serversChunkSize: 500,
    });
  }

  makeList(config: IServerListConfig) {
    this.sendToWorker('applyListConfig', config);
  }

  setPinnedConfig(config: IPinnedServersConfig): void {
    this.sendToWorker('setPinnedServersConfig', config);
  }

  private readonly handleWorkerMessage = (event: MessageEvent) => {
    const message = event.data;

    if (!isObject<ServerResponsePayload>(message)) {
      return;
    }

    switch (message.type) {
      case 'allServersBegin':
        return this.serversFetchStartEvent.emit();
      case 'allServersChunk':
        return this.serversFetchChunkEvent.emit(message.data);
      case 'allServersEnd':
        return this.serversFetchEndEvent.emit(message.data);

      case 'list':
        return this.listEvent.emit(message.data[0], message.data[1]);
      case 'index':
        return this.indexEvent.emit(message.data);
    }
  };

  private sendToWorker<TMethod extends keyof ServersWorker>(
    method: TMethod,
    ...args: Parameters<ServersWorker[TMethod]>
  ): void {
    this.worker.postMessage({
      type: method,
      data: args,
    });
  }
}
