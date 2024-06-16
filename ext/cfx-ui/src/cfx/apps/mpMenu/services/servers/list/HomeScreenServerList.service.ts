import { inject, injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { IIntlService } from 'cfx/common/services/intl/intl.service';
import { isServerOffline } from 'cfx/common/services/servers/helpers';
import { reviveServerListConfig } from 'cfx/common/services/servers/lists/ServerListConfigController';
import { ServerListSortDir, ServersListSortBy, ServersListType } from 'cfx/common/services/servers/lists/types';
import { IServersStorageService } from 'cfx/common/services/servers/serversStorage.service';
import { IServerView } from 'cfx/common/services/servers/types';

import { MpMenuServersService } from '../servers.mpMenu';

export const MAX_TOP_SERVERS_COUNT = 10;

export const IDLE_TIMEOUT = 60 * 1000; // 60 seconds

export function getTopRegionServerOnScreenTime(index: number): number {
  let time = 5;

  switch (index) {
    case 0: {
      time = 30;
      break;
    }

    case 1: {
      time = 10;
      break;
    }
    case 2: {
      time = 10;
      break;
    }

    case 3: {
      time = 8;
      break;
    }

    default: {
      time = 5;
      break;
    }
  }

  return time * 1000;
}

export function useHomeScreenServerList() {
  return useService(HomeScreenServerListService);
}

export function registerHomeScreenServerList(container: ServicesContainer) {
  container.register(HomeScreenServerListService);

  registerAppContribution(container, HomeScreenServerListService);
}

@injectable()
export class HomeScreenServerListService implements AppContribution {
  private _topRegionServers: IServerView[] = [];
  public get topRegionServers(): IServerView[] {
    return this._topRegionServers;
  }
  private set topRegionServers(topServers: IServerView[]) {
    this._topRegionServers = topServers;
  }

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
      _topServers: observable.ref,
    });
  }

  init() {
    this.serversService.listSource.onList(ServersListType.RegionalTop, this.acceptServerList);
    this.serversService.listSource.makeList(
      reviveServerListConfig({
        type: ServersListType.RegionalTop,
        locales: {
          [this.intlService.systemLocale]: true,
        },
        sortBy: ServersListSortBy.Boosts,
        sortDir: ServerListSortDir.Desc,
      }),
    );
  }

  private readonly acceptServerList = (serverIds: string[]) => {
    if (this._topRegionServers.length === MAX_TOP_SERVERS_COUNT) {
      return;
    }

    const servers: IServerView[] = [];

    for (const serverId of serverIds) {
      if (servers.length === MAX_TOP_SERVERS_COUNT) {
        break;
      }

      const server = this.serversService.getServer(serverId);

      if (!server) {
        continue;
      }

      if (isServerOffline(server)) {
        continue;
      }

      if (server.private) {
        continue;
      }

      servers.push(server);
    }

    this.topRegionServers = servers;
  };

  get lastConnectedServer() {
    const lastServers = this.serversStorageService.getLastServers();

    if (!lastServers.length) {
      return;
    }

    const {
      address,
    } = lastServers[0];

    if (!address) {
      return;
    }

    return this.serversService.getServer(address);
  }
}
