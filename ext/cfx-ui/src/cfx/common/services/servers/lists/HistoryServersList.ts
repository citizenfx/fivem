import { IServersList } from "./types";
import { IHistoryServer, IServerView } from "../types";
import { makeAutoObservable, observable } from "mobx";
import { historyServer2ServerView } from "../transformers";
import { parseServerAddress } from "../utils";
import { IServersStorageService } from "../serversStorage.service";
import { getServerIconThumbnailURL } from "../icon";
import { IServersService } from "../servers.service";

export class HistoryServersList implements IServersList {
  private get historyServers(): IHistoryServer[] { return this.serversStorageService.getLastServers() }

  private _serversLastConnectedAt: Record<string, Date> = this.historyServers.reduce((acc, { address, time }) => {
    acc[address] = time;

    return acc;
  }, {});

  get sequence(): string[] {
    return this.historyServers.map(({ address }) => address).reverse();
  }

  constructor(
    protected readonly serversStorageService: IServersStorageService,
    private readonly resolveServer: IServersService['loadServerLiveData']
  ) {
    makeAutoObservable(this);

    // Resolve all history entries
    for (const historyServer of this.historyServers) {
      this.resolveHistoryServer(historyServer);
    }
  }

  getLastConnectedAt(address: string): Date | undefined {
    return this._serversLastConnectedAt[address];
  }

  async add(server: IServerView) {
    this.addHistoryServer(await this.serverView2HistoryServer(server));
  }

  addHistoryServer(historyServer: IHistoryServer) {
    const newHistoryServers = [
      ...this.historyServers.filter(({ address }) => address !== historyServer.address),
      historyServer,
    ];

    this._serversLastConnectedAt[historyServer.address] = historyServer.time;

    this.serversStorageService.setLastServers(newHistoryServers);
  }

  private async resolveHistoryServer(historyServer: IHistoryServer) {
    if (!parseServerAddress(historyServer.address)) {
      return;
    }

    const server = await this.resolveServer(historyServer2ServerView(historyServer));

    if (historyServer.address !== server.address) {
      this._serversLastConnectedAt[server.address] = historyServer.time;
    }
  }

  async serverView2HistoryServer(server: IServerView, overrides: Partial<IHistoryServer> = {}): Promise<IHistoryServer> {
    const thumbnail = overrides.icon || await getServerIconThumbnailURL(server.address);

    return {
      address: server.address,
      hostname: server.projectName,
      title: '',
      time: new Date(),
      icon: thumbnail,
      rawIcon: thumbnail,
      vars: server.rawVariables,
      token: server.licenseKeyToken || '',

      ...overrides,
    };
  }

  refresh(): void {

  }
}
