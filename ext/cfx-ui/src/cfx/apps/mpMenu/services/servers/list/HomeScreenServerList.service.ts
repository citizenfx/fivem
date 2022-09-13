import { ServicesContainer, useService } from "cfx/base/servicesContainer";
import { IIntlService } from "cfx/common/services/intl/intl.service";
import { reviveServerListConfig } from "cfx/common/services/servers/lists/ServerListConfigController";
import { ServersListType } from "cfx/common/services/servers/lists/types";
import { IServersStorageService } from "cfx/common/services/servers/serversStorage.service";
import { IServerView } from "cfx/common/services/servers/types";
import { reverseArray } from "cfx/utils/array";
import { inject, injectable } from "inversify";
import { makeAutoObservable, observable } from "mobx";
import { MpMenuServersService } from "../servers.mpMenu";

export const MAX_TOP_SERVERS = 10;

export function getTopRegionServerOnScreenTime(index: number): number {
  let time = 5;

  switch (index) {
    case 0: { time = 30; break; }

    case 1: { time = 10; break; }
    case 2: { time = 10; break; }

    case 3: { time = 8; break; }

    default: { time = 5; break; }
  }

  return time * 1000;
}

export function useHomeScreenServerList() {
  return useService(HomeScreenServerListService);
}

export function registerHomeScreenServerList(container: ServicesContainer) {
  container.register(HomeScreenServerListService);
}

@injectable()
export class HomeScreenServerListService {
  private _allServersSequence: string[] = [];
  private get allServersSequence() { return this._allServersSequence }
  private set allServersSequence(seq: string[]) { this._allServersSequence = seq }

  private _historyServersSequence: string[] = [];
  private get historyServersSequence() { return this._historyServersSequence }
  private set historyServersSequence(seq: string[]) { this._historyServersSequence = seq }

  private _favoriteServersSequence: string[] = [];
  private get favoriteServersSequence() { return this._favoriteServersSequence }
  private set favoriteServersSequence(seq: string[]) { this._favoriteServersSequence = seq }

  constructor(
    @inject(MpMenuServersService)
    protected readonly serversService: MpMenuServersService,
    @inject(IIntlService)
    protected readonly intlService: IIntlService,
    @inject(IServersStorageService)
    protected readonly serversStorageService: IServersStorageService,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _allServersSequence: observable.ref,
      _historyServersSequence: observable.ref,
      _favoriteServerSequence: observable.ref,
    });

    this.serversStorageService.onFavoriteServers((seq) => this.favoriteServersSequence = reverseArray(seq));
    this.historyServersSequence = this.serversStorageService.getLastServers().map((server) => server.address);

    this.serversService.listSource.onList(ServersListType.PersonalizedServerList, (seq) => this.allServersSequence = seq);
    this.serversService.listSource.makeList(reviveServerListConfig({
      type: ServersListType.PersonalizedServerList,
      locales: {
        [this.intlService.systemLocale]: true,
      },
    }));
  }

  get topRegionServers() {
    const servers: IServerView[] = [];

    for (const serverId of this.allServersSequence) {
      if (servers.length === MAX_TOP_SERVERS) {
        return servers;
      }

      const server = this.serversService.getServer(serverId);
      if (!server) {
        continue;
      }

      if (server.offline) {
        continue;
      }

      servers.push(server);
    }

    return servers;
  }

  get lastConnectedServer() {
    const serverId = this.historyServersSequence[0];
    if (!serverId) {
      return;
    }

    return this.serversService.getServer(serverId);
  }
}
