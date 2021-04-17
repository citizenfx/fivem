import { inject, injectable } from "inversify";
import { AssetInterface } from "assets/core/asset-interface";
import { DisposableObject } from "backend/disposable-container";
import { FsWatcherEventType } from "backend/fs/fs-watcher";
import { FilesystemEntry } from "shared/api.types";
import { isChildAssetPath, isParentAssetPath } from "utils/project";
import { ProjectAssetManagers } from "./project-asset-managers";
import { LogService } from "backend/logger/log-service";
import { ApiClient } from "backend/api/api-client";
import { assetApi } from "shared/api.events";
import { AssetType } from "shared/asset.types";
import { Ticker } from "backend/ticker";

@injectable()
export class ProjectAssets implements DisposableObject {
  @inject(LogService)
  protected logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ProjectAssetManagers)
  protected readonly projectAssetManagers: ProjectAssetManagers;

  private assets: Record<string, AssetInterface> = Object.create(null);

  private assetTypeSetTicker = new Ticker();
  private pendingAssetTypesSet: Record<string, AssetType | void> | void;

  get(assetPath: string): AssetInterface {
    return this.assets[assetPath];
  }

  getAllPaths(): string[] {
    return Object.keys(this.assets);
  }

  load(assetEntry: FilesystemEntry): boolean {
    for (const manager of this.projectAssetManagers.getAll()) {
      const asset = manager.loadAsset(assetEntry);

      if (asset) {
        this.assets[assetEntry.path] = asset;
        this.setAssetType(assetEntry.path, asset.type);
        return true;
      }
    }

    return false;
  }

  async dispose() {
    for (const asset of Object.values(this.assets)) {
      this.setAssetType(asset.getPath(), undefined);
      await asset.dispose?.();
    }
  }

  async release(assetPath: string) {
    const asset = this.assets[assetPath];
    if (!asset) {
      return;
    }

    for (const asset of this.findAssetWithChildren(assetPath)) {
      this.setAssetType(asset.getPath(), undefined);
      delete this.assets[assetPath];
      await asset.dispose?.();
    }
  }

  fsUpdate(type: FsWatcherEventType, entryPath: string, entry: FilesystemEntry | null) {
    for (const asset of this.findAssetWithParents(entryPath)) {
      asset.onFsUpdate?.(type, entry, entryPath);
    }
  }

  private setAssetType(assetPath: string, assetType: AssetType | void) {
    if (!this.pendingAssetTypesSet) {
      this.pendingAssetTypesSet = {};
    }

    this.logService.log('Queueing asset type set', { assetPath, assetType });

    this.pendingAssetTypesSet[assetPath] = assetType;

    this.assetTypeSetTicker.whenTickEnds(() => {
      this.logService.log('Setting asset types', this.pendingAssetTypesSet);
      this.apiClient.emit(assetApi.setType, this.pendingAssetTypesSet);
      this.pendingAssetTypesSet = undefined;
    });
  }

  private *findAssetWithChildren(entryPath: string): IterableIterator<AssetInterface> {
    for (const [assetPath, asset] of Object.entries(this.assets)) {
      if (assetPath === entryPath) {
        yield asset;
        continue;
      }

      if (isChildAssetPath(entryPath, assetPath)) {
        yield asset;
      }
    }
  }

  private *findAssetWithParents(entryPath: string): IterableIterator<AssetInterface> {
    for (const [assetPath, asset] of Object.entries(this.assets)) {
      if (assetPath === entryPath) {
        yield asset;
        continue;
      }

      if (isParentAssetPath(entryPath, assetPath)) {
        yield asset;
      }
    }
  }
}
