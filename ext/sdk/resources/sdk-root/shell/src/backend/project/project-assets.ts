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
  private pendingAssetDefsSet: Record<string, any> | void;

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
        this.setAssetInformation(assetEntry.path, asset.type, asset.getDefinition?.() ?? {});
        return true;
      }
    }

    return false;
  }

  async dispose() {
    for (const asset of Object.values(this.assets)) {
      this.setAssetInformation(asset.getPath(), undefined, undefined);
      await asset.dispose?.();
    }
  }

  async release(assetPath: string) {
    const asset = this.assets[assetPath];
    if (!asset) {
      return;
    }

    for (const asset of this.findAssetWithChildren(assetPath)) {
      this.setAssetInformation(asset.getPath(), undefined, undefined);
      delete this.assets[assetPath];
      await asset.dispose?.();
    }
  }

  fsUpdate(type: FsWatcherEventType, entryPath: string, entry: FilesystemEntry | null) {
    for (const asset of this.findAssetWithParents(entryPath)) {
      asset.onFsUpdate?.(type, entry, entryPath);
    }
  }

  resolveMetadata(asset: AssetInterface) {
    this.setAssetInformation(asset.getPath(), asset.type, asset.getDefinition?.() ?? {});
  }

  private setAssetInformation(assetPath: string, assetType: AssetType | void, assetDef: any) {
    if (!this.pendingAssetTypesSet) {
      this.pendingAssetTypesSet = {};
    }

    if (!this.pendingAssetDefsSet) {
      this.pendingAssetDefsSet = {};
    }

    this.logService.log('Queueing asset info set', { assetPath, assetType, assetDef });

    this.pendingAssetTypesSet[assetPath] = assetType;
    this.pendingAssetDefsSet[assetPath] = assetDef;

    this.assetTypeSetTicker.whenTickEnds(() => {
      this.logService.log('Setting asset infos', this.pendingAssetTypesSet, this.pendingAssetDefsSet);
      this.apiClient.emit(assetApi.setType, this.pendingAssetTypesSet);
      this.apiClient.emit(assetApi.setDefinition, this.pendingAssetDefsSet);
      this.pendingAssetTypesSet = undefined;
      this.pendingAssetDefsSet = undefined;
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
