import { makeAutoObservable, observable } from "mobx";
import { IServerListSource } from "../source/types";
import { ServerListConfigController } from "./ServerListConfigController";
import { IPartialServerListConfig, IServersList } from "./types";

export class BaseConfigurableServersList implements IServersList {
  private initialized = false;

  private _config: ServerListConfigController;
  public getConfig(): ServerListConfigController {
    return this._config;
  }

  private _list: string[] = [];
  public get sequence(): string[] {
    if (!this.initialized) {
      this.initialized = true;
      Promise.resolve().then(() => this.refresh());
    }

    return this._list;
  }
  private set sequence(list: string[]) { this._list = list }

  constructor(
    config: IPartialServerListConfig,
    source: IServerListSource,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error
      _source: false,
      _list: observable.ref,
    });

    source.onList(config.type, (list) => {
      this.sequence = list;
    });

    this._config = new ServerListConfigController({
      config,
      onChange: (config) => source.makeList(config),
    });
  }

  refresh() {
    this._config.refresh();
  }
}
