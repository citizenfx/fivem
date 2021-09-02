import { inject, injectable, named } from "inversify";
import { AssetInterface } from "assets/core/asset-interface";
import { DisposableObject } from "backend/disposable-container";
import { FilesystemEntry } from "shared/api.types";
import { isChildAssetPath, isParentAssetPath } from "utils/project";
import { LogService } from "backend/logger/log-service";
import { ApiClient } from "backend/api/api-client";
import { assetApi, projectApi } from "shared/api.events";
import { AssetImporterType, AssetMeta, assetMetaFileExt, AssetType } from "shared/asset.types";
import { Ticker } from "backend/ticker";
import { ProjectAssetManagers } from "../project-asset-managers";
import { APIRQ } from "shared/api.requests";
import { FsService } from "backend/fs/fs-service";
import { ProjectRuntime } from "./project-runtime";
import { AssetImporterContribution } from "../asset/asset-importer-contribution";
import { ContributionProvider } from "backend/contribution-provider";
import { AssetManagerContribution } from "../asset/asset-manager-contribution";
import { ProjectAssetBaseConfig } from "shared/project.types";

interface Silentable {
  silent?: boolean,
}

export interface GetAssetMetaOptions extends Silentable {
}

@injectable()
export class ProjectAssetsRuntime implements DisposableObject {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ProjectAssetManagers)
  protected readonly assetManagers: ProjectAssetManagers;

  @inject(ContributionProvider) @named(AssetManagerContribution)
  protected readonly assetManagerContributions: ContributionProvider<AssetManagerContribution>;

  @inject(ContributionProvider) @named(AssetImporterContribution)
  protected readonly assetImporterContributions: ContributionProvider<AssetImporterContribution>;

  private rt: ProjectRuntime;

  private assets: Record<string, AssetInterface> = Object.create(null);

  private assetTypeSetTicker = new Ticker();
  private pendingAssetTypesSet: Record<string, AssetType | void> | void;
  private pendingAssetDefsSet: Record<string, any> | void;

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    rt.registerEntryMetaContribution('assetMeta', (entryPath: string) => this.getMeta(entryPath, { silent: true }));

    rt.fsEntry.addListener((entry: FilesystemEntry) => {
      this.assetManagers.handleFSEntry(entry);

      const existingAsset = this.get(entry.path);
      if (existingAsset) {
        existingAsset.setEntry?.(entry);
      } else {
        this.load(entry);
      }
    });

    rt.fsUpdate.addListener(({ type, entry, path }) => {
      for (const asset of this.findAssetWithParents(path)) {
        asset.handleFSUpdate?.(type, entry, path);
      }
    });

    rt.fsEntryRenamed.addListener(({ oldPath }) => {
      if (oldPath) {
        this.release(oldPath);
      }
    });

    rt.fsEntryDeleted.addListener((entryPath) => {
      this.release(entryPath);
    });
  }

  get(assetPath: string): AssetInterface {
    return this.assets[assetPath];
  }

  has(assetPath: string): boolean {
    return !!this.assets[assetPath];
  }

  getAllPaths(): string[] {
    return Object.keys(this.assets);
  }

  load(assetEntry: FilesystemEntry): boolean {
    for (const manager of this.assetManagers.getAll()) {
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

  getConfig(assetPath: string): ProjectAssetBaseConfig {
    const assetRelativePath = this.fsService.relativePath(this.rt.state.path, assetPath);

    return this.rt.getManifest().assets[assetRelativePath] || { enabled: false };
  }

  async setConfig(request: APIRQ.ProjectSetAssetConfig) {
    const assetRelativePath = this.fsService.relativePath(this.rt.state.path, request.assetPath);

    const newConfig = {
      ...this.getConfig(request.assetPath),
      ...request.config,
    };

    this.rt.applyManifest((manifest) => {
      manifest.assets[assetRelativePath] = newConfig;
    });

    this.rt.emitAssetConfigChanged(request.assetPath, newConfig);

    this.apiClient.emit(assetApi.setConfig, [request.assetPath, newConfig]);
  }

  deleteConfig(assetPath: string) {
    const assetRelativePath = this.fsService.relativePath(this.rt.state.path, assetPath);

    this.rt.applyManifest((manifest) => {
      delete manifest.assets[assetRelativePath];
    });
  }

  getMetaPath(assetPath: string): string {
    const assetName = this.fsService.basename(assetPath);
    const assetMetaFilename = assetName + assetMetaFileExt;

    return this.fsService.joinPath(this.fsService.dirname(assetPath), assetMetaFilename);;
  }

  async getMeta(assetPath: string, options?: GetAssetMetaOptions): Promise<AssetMeta | null> {
    const assetMetaFilepath = this.getMetaPath(assetPath);

    try {
      await this.fsService.stat(assetMetaFilepath);

      return await this.fsService.readFileJson(assetMetaFilepath);
    } catch (e) {
      if (!options?.silent) {
        console.error('Error reading asset meta from file', {
          projectPath: this.rt.state.path,
          assetMetaFilepath,
        }, e);
      }

      return null;
    }
  }

  async setMeta(assetPath: string, assetMeta: AssetMeta) {
    const assetMetaFilepath = this.getMetaPath(assetPath);

    await this.fsService.writeFileJson(assetMetaFilepath, assetMeta);

    this.rt.forceFSScan(assetPath);
  }

  async hasMeta(assetPath: string) {
    return !!(await this.fsService.statSafe(this.getMetaPath(assetPath)));
  }

  protected getImporter(importerType: AssetImporterType): AssetImporterContribution {
    try {
      return this.assetImporterContributions.getTagged('importerType', importerType);
    } catch (e) {
      throw new Error(`No asset importer contribution of type ${importerType}`);
    }
  }

  async import(request: APIRQ.AssetImport) {
    return this.getImporter(request.importerType).importAsset(request);
  }

  async create(request: APIRQ.AssetCreate) {
    return this.assetManagers.get(request.assetType).createAsset(request);
  }

  async rename(request: APIRQ.RenameEntry) {
    const { newName: newAssetName, entryPath: assetPath } = request;

    return this.move({
      sourcePath: assetPath,
      targetPath: this.fsService.joinPath(this.fsService.dirname(assetPath), newAssetName),
    });
  }

  async move(request: APIRQ.MoveEntry) {
    const {
      sourcePath: assetPath,
      targetPath,
    } = request;

    const newAssetPath = this.fsService.joinPath(targetPath, this.fsService.basename(assetPath));

    await this.release(assetPath);

    const assetConfig = this.getConfig(assetPath);
    if (assetConfig) {
      this.deleteConfig(assetPath);

      this.setConfig({
        assetPath: newAssetPath,
        config: assetConfig,
      });
    }


    const promises = [
      this.fsService.rename(assetPath, newAssetPath),
    ];

    const oldAssetMetaPath = this.getMetaPath(assetPath);
    const newAssetMetaPath = this.getMetaPath(newAssetPath);

    if (await this.fsService.statSafe(oldAssetMetaPath)) {
      promises.push(
        this.fsService.rename(oldAssetMetaPath, newAssetMetaPath),
      );
    }

    await Promise.all(promises);

    this.rt.fsEntryMoved.emit({
      oldPath: assetPath,
      newPath: newAssetPath,
    });
  }

  async delete(request: APIRQ.DeleteEntry): Promise<APIRQ.DeleteEntryResponse> {
    const { entryPath: assetPath } = request;
    const assetMetaPath = this.getMetaPath(assetPath);
    const hasAssetMeta = !!(await this.fsService.statSafe(assetMetaPath));

    const assetStat = await this.fsService.statSafe(assetPath);
    if (!assetStat) {
      return APIRQ.DeleteEntryResponse.Ok;
    }

    await this.release(assetPath);

    try {
      if (request.hardDelete) {
        await this.fsService.rimraf(assetPath);
        if (hasAssetMeta) {
          await this.fsService.rimraf(assetMetaPath);
        }
      } else {
        try {
          await this.fsService.moveToTrashBin(assetPath);
          if (hasAssetMeta) {
            await this.fsService.moveToTrashBin(assetMetaPath);
          }
        } catch (e) {
          this.logService.log(`Failed to recycle asset: ${e.toString()}`);
          this.rt.forceFSScan(assetPath);
          return APIRQ.DeleteEntryResponse.FailedToRecycle;
        }
      }

      return APIRQ.DeleteEntryResponse.Ok;
    } catch (e) {
      this.rt.forceFSScan(assetPath);
      throw e;
    } finally {
      if (assetStat.isDirectory()) {
        this.apiClient.emit(projectApi.freePendingFolderDeletion, assetPath);
      }
    }
  }

  refreshInfo(assetPath: string) {
    const asset = this.get(assetPath);
    if (asset) {
      this.setAssetInformation(assetPath, asset.type, asset.getDefinition?.() ?? {});
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
