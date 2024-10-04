import { GameName } from 'cfx/base/game';
import { shouldPrioritizePinnedServers } from 'cfx/base/serverUtils';

import { AutocompleteIndexer } from './autocomplete';
import { filterList } from './listFilter';
import { sortList } from './listSorter';
import { IAutocompleteIndex, IListableServerView } from './types';
import { getAllMasterListServers } from './utils/fetchers';
import { IServerListConfig } from '../lists/types';
import { serverView2ListableServerView } from '../transformers';
import { IPinnedServersConfig, IServerView } from '../types';

function postMessageToMainThread(type: string, data: any) {
  try {
    postMessage({
      type,
      data,
    });
  } catch (e) {
    console.warn(e, type, data);
  }
}

export const ServerResponses = defineEvents(postMessageToMainThread)
  .add<'allServersBegin'>()
  .add<'allServersChunk', IServerView[]>()
  .add<'allServersEnd', IServerView[]>()
  .add<'allServersError', string>()
  .add<'index', IAutocompleteIndex>()
  .add<'list', [string, string[]]>();

export type ServerResponsePayload = typeof ServerResponses.TPayloads;

export interface IServersWorkerInitOptions {
  gameName: GameName;

  serversChunkSize: number;
}

export class ServersWorker {
  private fetching = true;

  private serversIndex = new AutocompleteIndexer();

  private listableServersMap: Record<string, IListableServerView> = {};

  // Sorted server IDs lists to use for further filtering
  private sortedLists: Record<string, string[]> = {};

  private listsRegistry: Record<string, { id: number; config: IServerListConfig | null }> = {};

  private gameName = GameName.FiveM;

  private serversChunkSize = 500;

  private pinnedServersConfig: IPinnedServersConfig | null = null;

  constructor() {
    onmessage = (event: MessageEvent) => {
      const {
        type,
        data,
      } = event.data;

      if (typeof this[type] === 'function') {
        this[type](...data);
      }
    };
  }

  async init(options: IServersWorkerInitOptions) {
    this.gameName = options.gameName;

    this.serversChunkSize = options.serversChunkSize;

    this.refresh();
  }

  setPinnedServersConfig(config: IPinnedServersConfig | null) {
    this.pinnedServersConfig = config;
  }

  applyListConfig(config: IServerListConfig) {
    const listType = config.type;

    if (!this.listsRegistry[listType]) {
      this.listsRegistry[listType] = {
        id: 0,
        config,
      };
    } else {
      this.listsRegistry[listType].config = config;
      this.listsRegistry[listType].id++;
    }

    if (!this.fetching) {
      this.filterAndSendList(config.type);
    }
  }

  async refresh() {
    this.fetching = true;
    ServerResponses.send('allServersBegin');

    let serversBuffer: IServerView[] = [];

    const sendBuffer = () => {
      if (serversBuffer.length === 0) {
        return;
      }

      ServerResponses.send('allServersChunk', serversBuffer);
      serversBuffer = [];

      this.reapplyListConfigs();
    };
    const chunkTimer = setInterval(sendBuffer, 500);

    try {
      await getAllMasterListServers(this.gameName, (server: IServerView) => {
        this.serversIndex.add(server);

        this.listableServersMap[server.id] = serverView2ListableServerView(server);

        serversBuffer.push(server);

        if (serversBuffer.length === this.serversChunkSize) {
          sendBuffer();
        }
      });

      clearInterval(chunkTimer);

      ServerResponses.send('allServersEnd', serversBuffer);
      ServerResponses.send('index', this.serversIndex.getIndex());

      this.reapplyListConfigs();
    } catch (e) {
      console.error(e);

      ServerResponses.send('allServersError', e.message);
    } finally {
      this.fetching = false;
    }
  }

  private reapplyListConfigs() {
    for (const {
      config,
    } of Object.values(this.listsRegistry)) {
      if (config) {
        this.createdSortedList(config);
        this.filterAndSendList(config.type);
      }
    }
  }

  private filterAndSendList(listType: string) {
    const list = this.listsRegistry[listType];

    if (!list || !list.config) {
      return;
    }

    this.sendList(
      list.config,
      list.id,
      filterList(this.listableServersMap, this.getSortedList(list.config), list.config),
    );
  }

  private getSortedList(config: IServerListConfig): string[] {
    const listHash = getSortedListHash(config);

    let sortedList = this.sortedLists[listHash];

    if (!sortedList) {
      sortedList = this.createdSortedList(config);
    }

    return sortedList;
  }

  private createdSortedList(config: IServerListConfig): string[] {
    const hash = getSortedListHash(config);
    this.sortedLists[hash] = sortList(this.listableServersMap, this.pinnedServersConfig, config);

    return this.sortedLists[hash];
  }

  private async sendList(config: IServerListConfig, id: number, list: string[]) {
    await Promise.resolve();

    if (this.listsRegistry[config.type].id !== id) {
      console.log('Dropping list as not most recent', config.type, id, this.listsRegistry[config.type]);

      return;
    }

    ServerResponses.send('list', [config.type, list]);
  }
}

// eslint-disable-next-line no-new
new ServersWorker();

export interface Definition<TMap extends Record<string, any>, TPL extends { type: string; data: any }> {
  TEvents: TMap;
  TPayloads: TPL;

  send<T extends keyof TMap>(type: T, data?: TMap[T]): void;
  add<Type extends string, Data = void>(): Definition<TMap & Record<Type, Data>, TPL | { type: Type; data: Data }>;
}

export function defineEvents(
  send: (type: string, data: any) => void,
): Definition<object, { type: never; data: never }> {
  const definitions = {
    send,
    add: () => definitions,
  } as any;

  return definitions;
}

function getSortedListHash(config: IServerListConfig): string {
  let hash = `${config.sortBy}:${config.sortDir}`;

  if (shouldPrioritizePinnedServers(config)) {
    hash += ':pinsOnTop';
  }

  return hash;
}
