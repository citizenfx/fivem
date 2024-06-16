import { inject, injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';
import { computedFn } from 'mobx-utils';

import { CurrentGameName } from 'cfx/base/gameRuntime';
import { getListServerTags, getPinnedServersList } from 'cfx/base/serverUtils';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { scopedLogger, ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { BaseConfigurableServersList } from 'cfx/common/services/servers/lists/BaseConfigurableServersList';
import { FavoriteServersList } from 'cfx/common/services/servers/lists/FavoriteServersList';
import { HistoryServersList } from 'cfx/common/services/servers/lists/HistoryServersList';
import { IServersList, ServersListType } from 'cfx/common/services/servers/lists/types';
import { parseServerAddress } from 'cfx/common/services/servers/serverAddressParser';
import { mergeServers } from 'cfx/common/services/servers/serverMerger';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServersStorageService } from 'cfx/common/services/servers/serversStorage.service';
import { IAutocompleteIndex, IServerListSource } from 'cfx/common/services/servers/source/types';
import { getMasterListServer } from 'cfx/common/services/servers/source/utils/fetchers';
import { WorkerSource } from 'cfx/common/services/servers/source/WorkerSource';
import { serverAddress2ServerView } from 'cfx/common/services/servers/transformers';
import {
  IPinnedServersCollection,
  IPinnedServersConfig,
  IServerView,
  ServerViewDetailsLevel,
} from 'cfx/common/services/servers/types';
import { uniqueArray } from 'cfx/utils/array';
import { fetcher } from 'cfx/utils/fetcher';
import { isObject } from 'cfx/utils/object';

import { getServerForAddress } from './source/fetchers';

const IMpMenuServersServiceInit = Symbol('MpMenuServersServiceInit');
export interface IMpMenuServersServiceInit {
  listTypes: ServersListType[];
}

export function registerMpMenuServersService(container: ServicesContainer, init: IMpMenuServersServiceInit) {
  container.registerConstant(IMpMenuServersServiceInit, init);

  container.registerImpl(IServersService, MpMenuServersService);

  registerAppContribution(container, MpMenuServersService);
}

@injectable()
export class MpMenuServersService implements IServersService, AppContribution {
  @inject(IMpMenuServersServiceInit)
  protected readonly initObject: IMpMenuServersServiceInit;

  @inject(IServersStorageService)
  protected readonly serversStorageService: IServersStorageService;

  @scopedLogger('MpMenuServersService')
  protected readonly logger: ScopedLogger;

  private _topLocaleServerId: string = '';
  public get topLocaleServerId(): string {
    return this._topLocaleServerId;
  }
  private set topLocaleServerId(promotedServerId: string) {
    this._topLocaleServerId = promotedServerId;
  }

  private _serversListLoading: boolean = true;
  public get serversListLoading(): boolean {
    return this._serversListLoading;
  }
  private set serversListLoading(serversLoading: boolean) {
    this._serversListLoading = serversLoading;
  }

  private serverDetailsLoadRequested: Record<string, boolean> = {};

  private _pinnedServersConfig: Readonly<IPinnedServersConfig> | null = null;
  public get pinnedServersConfig(): Readonly<IPinnedServersConfig> | null {
    return this._pinnedServersConfig;
  }
  private set pinnedServersConfig(pinnedServersConfig: Readonly<IPinnedServersConfig> | null) {
    this._pinnedServersConfig = pinnedServersConfig;
  }

  public get pinnedServers(): string[] {
    return getPinnedServersList(this._pinnedServersConfig, this.getServer);
  }

  private _serverIdsAliases: Record<string, string> = {};

  private _serverIdsAliasesReverse: Record<string, string> = {};

  private _servers: Map<string, IServerView> = new Map();

  private _serversLoading: Record<string, ServerViewDetailsLevel | false> = {};

  private _serversIndex: IAutocompleteIndex | null = null;
  public get autocompleteIndex(): IAutocompleteIndex | null {
    return this._serversIndex;
  }
  private set autocompleteIndex(serversIndex: IAutocompleteIndex | null) {
    this._serversIndex = serversIndex;
  }

  readonly listSource: IServerListSource;

  private lists: Partial<Record<ServersListType, IServersList>>;

  public listTypes: ServersListType[] = [];

  private _totalServersCount: number = 0;
  public get totalServersCount(): number {
    return this._totalServersCount;
  }
  private set totalServersCount(totalServersCount: number) {
    this._totalServersCount = totalServersCount;
  }

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _servers: observable.shallow,
      _serversIndex: observable.ref,
      serverDetailsLoadRequested: false,
    });

    this.listSource = new WorkerSource();

    this.listSource.onIndex((index) => {
      this.autocompleteIndex = index;
    });

    this.listSource.onServersFetchStart(() => {
      this.totalServersCount = 0;
      this.serversListLoading = true;
    });
    this.listSource.onServersFetchChunk((chunk) => this.populateServersFromChunk(chunk));
    this.listSource.onServersFetchEnd((chunk) => {
      this.populateServersFromChunk(chunk);
      this.serversListLoading = false;
    });

    // FIXME proper error handling
    this.listSource.onServersFetchError((error) => console.error(error));

    this.lists = {};
  }

  async init() {
    this.listSource.init();

    this.listTypes = this.initObject.listTypes;

    for (const listType of this.initObject.listTypes) {
      this.lists[listType] = this.initList(listType);
    }

    await this.loadPinnedServersConfig();
  }

  getList(type: ServersListType): IServersList | undefined {
    return this.lists[type];
  }

  getAllList(): BaseConfigurableServersList | undefined {
    return this.lists[ServersListType.All] as any;
  }

  getFavoriteList(): FavoriteServersList | undefined {
    return this.lists[ServersListType.Favorites] as any;
  }

  getHistoryList(): HistoryServersList | undefined {
    return this.lists[ServersListType.History] as any;
  }

  private setServerIdAlias(id: string, newId: string) {
    if (id === newId) {
      return;
    }

    this._serverIdsAliases[id] = newId;
    this._serverIdsAliasesReverse[newId] = id;
  }

  getRealServerId(id: string): string {
    return this._serverIdsAliases[id] || id;
  }

  getAllServerIds(id: string): string[] {
    return uniqueArray([id, this._serverIdsAliasesReverse[id], this._serverIdsAliases[id]].filter(Boolean));
  }

  private serversSet(id: string, server: IServerView) {
    this._servers.set(this.getRealServerId(id), server);
  }

  readonly getServer = (id: string): IServerView | undefined => this._servers.get(this.getRealServerId(id));

  isServerLoading(id: string, detailsLevel?: ServerViewDetailsLevel): boolean {
    if (typeof detailsLevel === 'undefined') {
      return this.serversListLoading;
    }

    const loadingState = this._serversLoading[id];

    if (!loadingState) {
      return false;
    }

    return loadingState >= detailsLevel;
  }

  private setServerLoadingState(id: string, state: ServerViewDetailsLevel | false) {
    this._serversLoading[id] = state;
  }

  async loadServerDetailedData(id: string) {
    if (this.serverDetailsLoadRequested[id]) {
      return;
    }

    this.serverDetailsLoadRequested[id] = true;

    this.setServerLoadingState(id, ServerViewDetailsLevel.MasterListFull);
    const server = await getMasterListServer(CurrentGameName, id);
    this.setServerLoadingState(id, false);

    if (server) {
      this.replaceServer(server);
    } else {
      this.logger.log(`Failed to load server (${id}) detailed data from master list`);

      // But if it was listed - mark as offline
      const listedServer = this.getServer(id);

      if (listedServer?.detailsLevel === ServerViewDetailsLevel.MasterList) {
        this.replaceServer({
          ...listedServer,
          offline: true,
        });
      }
    }
  }

  isServerPinned(address: string): boolean {
    return !!this.pinnedServersConfig?.pinnedServers.includes(address);
  }

  readonly getTagsForServer = computedFn((server: IServerView): string[] => getListServerTags(
    server,
    this.autocompleteIndex,
  ));

  serverAddressToServerView(address: string): IServerView | null {
    const parsedAddresss = parseServerAddress(address);

    if (!parsedAddresss) {
      return null;
    }

    return serverAddress2ServerView(parsedAddresss.address);
  }

  loadServerLiveData(server: IServerView): Promise<IServerView>;

  loadServerLiveData(address: string): Promise<IServerView | null>;

  async loadServerLiveData(serverOrAddress: IServerView | string): Promise<IServerView | null> {
    let server: IServerView | null = null;

    if (typeof serverOrAddress === 'string') {
      server = this.serverAddressToServerView(serverOrAddress);
    } else {
      server = serverOrAddress;
    }

    // Can only be null if we've got the address and it is invalid
    if (!server) {
      return null;
    }

    if (server.detailsLevel >= ServerViewDetailsLevel.Live) {
      return server;
    }

    this.setServer(server.id, server);

    const resolvedServer = mergeServers(server, await getServerForAddress(server.id));

    this.setServer(resolvedServer.id, resolvedServer);
    this.setServerIdAlias(server.id, resolvedServer.id);

    if (resolvedServer.joinId) {
      this.setServer(resolvedServer.joinId, resolvedServer);
      this.setServerIdAlias(server.id, resolvedServer.joinId);
      this.setServerIdAlias(resolvedServer.id, resolvedServer.joinId);
    }

    return this.getServer(server.id)!;
  }

  /**
   * Literally just replaces server
   *
   * For smarter handling use ServersService.setServer()
   */
  replaceServer(server: IServerView) {
    this.serversSet(server.id, server);
  }

  setServer(serverId: string, server: IServerView) {
    const existingServer = this.getServer(serverId);

    const serverToSet = existingServer
      ? mergeServers(existingServer, server)
      : server;

    this.serversSet(serverId, serverToSet);
  }

  private initList(type: ServersListType): IServersList {
    switch (type) {
      case ServersListType.All: {
        return new BaseConfigurableServersList({ type: ServersListType.All }, this.listSource);
      }
      case ServersListType.History: {
        return new HistoryServersList(this.serversStorageService, (server) => this.loadServerLiveData(server));
      }
      case ServersListType.Favorites: {
        return new FavoriteServersList(
          this,
          this.serversStorageService,
          this.getServer,
          () => this._pinnedServersConfig,
        );
      }
      case ServersListType.Supporters: {
        return new BaseConfigurableServersList(
          { type: ServersListType.Supporters,
            onlyPremium: true },
          this.listSource,
        );
      }

      default: {
        throw new Error('Unknown servers list type');
      }
    }
  }

  private async loadPinnedServersConfig() {
    try {
      /**
       * Expected schema of `/pins.json` file:
       *
       * {
       *   noAdServer?: string | { title: string, ids: Array<string> },
       *   noAdServerId?: string, // will be ignored if there's a valid noAdServer
       *   pinnedServers?: Array<string>,
       *   pinIfEmpty?: boolean,
       * }
       *
       */
      const json = await fetcher.json('https://runtime.fivem.net/pins.json');

      const config: IPinnedServersConfig = {
        pinnedServers: [],
      };

      // Parsing `.noAdServer`,
      // starting with property supporting both single server join id and servers collection
      if (json.noAdServer) {
        const jsonNoAdServer: unknown = json.noAdServer;

        // Just one server join id
        if (typeof jsonNoAdServer === 'string') {
          config.featuredServer = {
            type: 'id',
            id: jsonNoAdServer,
          };
        // Servers collection
        } else if (isObject<Partial<IPinnedServersCollection>>(jsonNoAdServer)) {
          // `.title` and `.ids` are required
          if (typeof jsonNoAdServer.title === 'string' && Array.isArray(jsonNoAdServer.ids)) {
            const serverIds = jsonNoAdServer.ids.map((id) => String(id)).filter(Boolean);

            if (serverIds.length > 1) {
              config.featuredServer = {
                type: 'collection',
                collection: {
                  title: jsonNoAdServer.title,
                  ids: serverIds,
                },
              };
            // Fall back to single server join id if only one id is present
            } else if (serverIds.length === 1) {
              config.featuredServer = {
                type: 'id',
                id: String(serverIds[0]),
              };
            }
          }
        }
      }

      // Parsing old `.noAdServerId` if no featuredServer has been parsed yet
      if (json.noAdServerId && !config.featuredServer) {
        if (typeof json.noAdServerId === 'string') {
          config.featuredServer = {
            type: 'id',
            id: json.noAdServerId,
          };
        }
      }

      if (json.pinIfEmpty) {
        config.pinIfEmpty = Boolean(json.pinIfEmpty);
      }

      if (Array.isArray(json.pinnedServers)) {
        // TODO: the length is capped to 6 chars to filter out IP:port addresses
        config.pinnedServers = json.pinnedServers.filter(
          (address: unknown) => typeof address === 'string' && address.length === 6,
        );
      }

      this.listSource.setPinnedConfig(config);
      this.pinnedServersConfig = config;
    } catch (e) {
      this.logger.log('Failed to fetch pinned servers config');
      console.warn(e);
    }
  }

  private readonly populateServersFromChunk = (chunk: IServerView[]) => {
    for (const server of chunk) {
      this.totalServersCount++;
      this.setServer(server.id, server);
    }
  };
}
