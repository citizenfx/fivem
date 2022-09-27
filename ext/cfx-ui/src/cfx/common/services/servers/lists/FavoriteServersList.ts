import { makeAutoObservable } from "mobx";
import { IServersStorageService } from "../serversStorage.service";
import { filterList } from "../source/listFilter";
import { sortList } from "../source/listSorter";
import { IListableServerView } from "../source/types";
import { serverView2ListableServerView } from "../transformers";
import { IPinnedServersConfig, IServerView } from "../types";
import { ServerListConfigController } from "./ServerListConfigController";
import { IServersList, ServersListType } from "./types";

export class FavoriteServersList implements IServersList {
  private _favoriteServers: Record<string, true> = {};
  private set favoriteServers(servers: Record<string, true>) { this._favoriteServers = servers }

  private _config: ServerListConfigController;
  public getConfig(): ServerListConfigController {
    return this._config;
  }

  private get existingServers(): IServerView[] {
    return Object.keys(this._favoriteServers).map((id) => this.getServer(id)).filter(Boolean) as any;
  }

  public get sequence(): string[] {
    const listableFavoriteServers: Record<string, IListableServerView> = this.existingServers.reduce((acc, server) => {
      acc[server.id] = serverView2ListableServerView(server);

      return acc;
    }, {});

    const sortedList = sortList(
      listableFavoriteServers,
      this.getPinnedServersConfig(),
      this._config.get(),
    );

    return filterList(
      listableFavoriteServers,
      sortedList,
      this._config.get(),
    );
  }

  constructor(
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
      config: { type: ServersListType.Favorites },
    });

    this.serversStorageService.onFavoriteServers((list) => {
      this.favoriteServers = list.reduce((acc, id) => {
        acc[id] = true;

        return acc;
      }, {});
    });
  }

  refresh() {
    this._config.refresh();
  }

  isIn(id: string): boolean {
    return this._favoriteServers[id] || false;
  }

  toggleIn(id: string) {
    if (this._favoriteServers[id]) {
      delete this._favoriteServers[id];
    } else {
      this._favoriteServers[id] = true;
    }

    this.serversStorageService.setFavoriteServers(Object.keys(this._favoriteServers));
  }
}
