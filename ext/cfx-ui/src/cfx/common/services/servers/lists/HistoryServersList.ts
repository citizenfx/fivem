import { makeAutoObservable } from 'mobx';

import { IServersList } from './types';
import { createServerHistoricalIconURL } from '../icon';
import { parseServerAddress } from '../serverAddressParser';
import { IServersService } from '../servers.service';
import { IServersStorageService } from '../serversStorage.service';
import { historyServer2ServerView } from '../transformers';
import { IHistoryServer, IServerView } from '../types';

export class HistoryServersList implements IServersList {
  private _historyResolveRequested = false;

  private get historyServers(): IHistoryServer[] {
    return this.serversStorageService.getLastServers();
  }

  private _serversLastConnectedAt: Record<string, Date> = {};

  get sequence(): string[] {
    if (!this._historyResolveRequested) {
      this._historyResolveRequested = true;
      this.resolveAllButFirstHistoryServers();
    }

    return this.historyServers.map(({
      address,
    }) => address);
  }

  constructor(
    protected readonly serversStorageService: IServersStorageService,
    private readonly resolveServer: IServersService['loadServerLiveData'],
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _historyResolveRequested: false,
    });

    this.init();
  }

  private async init() {
    await this.serversStorageService.lastServersPopulated;

    // Populate "last connected at" data for already known servers
    for (const historyServer of this.historyServers) {
      this._serversLastConnectedAt[historyServer.address] = historyServer.time;
    }

    // Resolve first server immediately to let home screen show it, the rest will be resolved when needed
    const firstHistoryServer = this.historyServers[0];

    if (firstHistoryServer) {
      this.resolveHistoryServer(firstHistoryServer);
    }
  }

  private async resolveAllButFirstHistoryServers() {
    // Nothing to resolve if just one history server, it was already resolved in the init
    if (this.historyServers.length <= 1) {
      return;
    }

    for (let i = 1; i < this.historyServers.length; i++) {
      this.resolveHistoryServer(this.historyServers[i]);
    }
  }

  readonly clear = () => {
    this.serversStorageService.clearLastServers();
  };

  getLastConnectedAt(address: string): Date | undefined {
    return this._serversLastConnectedAt[address];
  }

  async add(server: IServerView) {
    this.addHistoryServer(await this.serverView2HistoryServer(server));
  }

  addHistoryServer(historyServer: IHistoryServer) {
    this._serversLastConnectedAt[historyServer.address] = historyServer.time;

    return this.serversStorageService.addLastServer(historyServer);
  }

  private async resolveHistoryServer(historyServer: IHistoryServer) {
    if (!parseServerAddress(historyServer.address)) {
      return;
    }

    const server = await this.resolveServer(historyServer2ServerView(historyServer));

    if (historyServer.address !== server.id) {
      this._serversLastConnectedAt[server.id] = historyServer.time;
    }
  }

  async serverView2HistoryServer(
    server: IServerView,
    overrides: Partial<{ icon: string; token: string; vars: Record<string, string> }> = {},
  ): Promise<IHistoryServer> {
    const {
      icon, ...restOverrides
    } = overrides;

    const thumbnail = icon || (await createServerHistoricalIconURL(server));

    return {
      address: server.historicalAddress || server.joinId || server.id,
      hostname: server.projectName,
      title: '',
      time: new Date(),
      rawIcon: thumbnail,
      vars: server.rawVariables,
      token: server.licenseKeyToken || '',

      ...restOverrides,
    };
  }

  refresh(): void {}
}
