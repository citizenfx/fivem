import { makeAutoObservable } from 'mobx';

import { ServerListConfigController } from './ServerListConfigController';
import { IServersList, ServerListSortDir, ServersListSortBy, ServersListType } from './types';
import { IServersService } from '../servers.service';
import { IServersStorageService } from '../serversStorage.service';
import { filterList } from '../source/listFilter';
import { sortList } from '../source/listSorter';
import { IListableServerView } from '../source/types';
import { serverView2ListableServerView } from '../transformers';
import { IPinnedServersConfig, IServerView } from '../types';

export class FavoriteServersList implements IServersList {
  private _favoriteServersMap: Record<string, true> = {};

  private set favoriteServersMap(favoriteServersMap: Record<string, true>) {
    this._favoriteServersMap = favoriteServersMap;
  }

  private _config: ServerListConfigController;

  public getConfig(): ServerListConfigController {
    return this._config;
  }

  private get existingServers(): IServerView[] {
    return Object.keys(this._favoriteServersMap)
      .map((id) => this.getServer(id))
      .filter(Boolean) as any;
  }

  public get sequence(): string[] {
    const listableFavoriteServers: Record<string, IListableServerView> = this.existingServers.reduce((acc, server) => {
      acc[server.id] = serverView2ListableServerView(server);

      return acc;
    }, {});

    const sortedList = sortList(listableFavoriteServers, this.getPinnedServersConfig(), this._config.get());

    return filterList(listableFavoriteServers, sortedList, this._config.get());
  }

  constructor(
    protected readonly serversService: IServersService,
    protected readonly serversStorageService: IServersStorageService,
    private readonly getServer: (id: string) => IServerView | undefined,
    private readonly getPinnedServersConfig: () => IPinnedServersConfig | null,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error
      getServer: false,
      getPinnedServersConfig: false,
    });

    this._config = new ServerListConfigController({
      config: {
        type: ServersListType.Favorites,
        sortBy: ServersListSortBy.Name,
        sortDir: ServerListSortDir.Asc,
      },
    });

    this.init();
  }

  private async init() {
    await this.serversStorageService.favoriteServersSequencePopulated;

    this.favoriteServersMap = this.serversStorageService.getFavoritesServersSequence().reduce((acc, id) => {
      acc[id] = true;

      return acc;
    }, {});
  }

  refresh() {
    this._config.refresh();
  }

  isIn(serverId: string): boolean {
    return this.serversService.getAllServerIds(serverId).some((id) => this._favoriteServersMap[id]);
  }

  toggleIn(serverId: string) {
    if (this._favoriteServersMap[serverId]) {
      this.serversService.getAllServerIds(serverId).forEach((id) => {
        delete this._favoriteServersMap[id];
      });
    } else {
      this._favoriteServersMap[serverId] = true;
    }

    this.serversStorageService.setFavoriteServers(Object.keys(this._favoriteServersMap));
  }
}
