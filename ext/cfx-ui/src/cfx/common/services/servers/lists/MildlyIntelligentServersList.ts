import { uniqueArray } from "cfx/utils/array";
import { makeAutoObservable, observable } from "mobx";
import { IServersService } from "../servers.service";
import { IServersStorageService } from "../serversStorage.service";
import { IServerListSource } from "../source/types";
import { reviveServerListConfig } from "./ServerListConfigController";
import { IServersList, ServersListType } from "./types";

export class MildlyIntelligentServersList implements IServersList {
  private _allServersSequence: string[] = [];
  private set allServersSequence(seq: string[]) { this._allServersSequence = seq }

  private _historyServersSequence: string[] = [];
  private get historyServersSequence() { return this._historyServersSequence }
  private set historyServersSequence(seq: string[]) { this._historyServersSequence = seq }

  private _favoriteServersSequence: string[] = [];
  private get favoriteServersSequence() { return this._favoriteServersSequence }
  private set favoriteServersSequence(seq: string[]) { this._favoriteServersSequence = seq }

  public get sequence() {
    const latestAndFavoriteServerIds = uniqueArray([
      this.historyServersSequence[this.historyServersSequence.length - 1],
      ...this.favoriteServersSequence,
    ]).filter((serverId) => {
      if (!serverId) {
        return false;
      }

      if (!this.serversService.getServer(serverId)) {
        return false;
      }

      return true;
    });

    const remainingServerIds = this._allServersSequence.filter((server) => !latestAndFavoriteServerIds.includes(server));

    return latestAndFavoriteServerIds.concat(remainingServerIds);
  }

  constructor(
    protected readonly serversService: IServersService,
    protected readonly serversStorageService: IServersStorageService,
    protected readonly listSource: IServerListSource,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _allServersSequence: observable.ref,
      _historyServersSequence: observable.ref,
      _favoriteServerSequence: observable.ref,
    });

    this.serversStorageService.onFavoriteServers((seq) => this.favoriteServersSequence = seq);
    this.historyServersSequence = this.serversStorageService.getLastServers().map((server) => server.address);

    this.listSource.onList(ServersListType.MildlyIntelligent, (seq) => this.allServersSequence = seq);
    this.listSource.makeList(reviveServerListConfig({
      type: ServersListType.MildlyIntelligent,
      // prioritizePinned: true,
    }));
  }

  refresh(): void {

  }
}
