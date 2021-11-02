import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { Api } from "fxdk/browser/Api";
import { ProjectApi } from "fxdk/project/common/project.api";
import { makeAutoObservable, observable } from "mobx";

enum State {
  NOT_LOADED,
  REQUESTED,
  LOADED,
}

export class AssetRuntimeDataStore implements IDisposableObject {
  private data: Record<string, any> = {};
  private dataState: Record<string, State> = {};

  private toDispose: Disposer;

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error as data is declared private
      data: observable.shallow,
    });

    this.toDispose = new Disposer();

    this.toDispose.register(Api.on(ProjectApi.AssetEndpoints.setAssetRuntimeData, this.setAssetRuntimeData));
    this.toDispose.register(Api.on(ProjectApi.AssetEndpoints.deleteAssetRuntimeData, this.deleteAssetRuntimeData));
  }

  dispose() {
    return dispose(this.toDispose);
  }

  get<T>(assetPath: string): T | undefined {
    this.dataState[assetPath] ??= State.NOT_LOADED;

    if (this.dataState[assetPath] === State.NOT_LOADED) {
      this.dataState[assetPath] = State.REQUESTED;
    }

    return this.data[assetPath];
  }

  private setAssetRuntimeData = ({ assetPath, data }) => {
    this.dataState[assetPath] = State.LOADED;
    this.data[assetPath] = data;
  };

  private deleteAssetRuntimeData = (assetPath: string) => {
    this.dataState[assetPath] = State.NOT_LOADED;
    delete this.data[assetPath];
  };
}
