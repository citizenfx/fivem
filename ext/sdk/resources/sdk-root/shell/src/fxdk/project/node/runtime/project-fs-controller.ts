import { FsService } from "backend/fs/fs-service";
import { dispose, Disposer, IDisposable, IDisposableObject } from "fxdk/base/disposable";
import { createFsWatcherEvent, FsWatcher, FsWatcherEvent, FsWatcherEventType } from 'backend/fs/fs-watcher';
import { FsUpdateKind, IFsEntry, IFsUpdate } from "../../common/project.types";
import { AssetMeta, assetMetaFileExt } from "shared/asset.types";
import { ProjectFsExtensions } from "../projectExtensions";
import { UniqueQueue } from "backend/queue";
import { ApiClient } from "backend/api/api-client";
import { ProjectFsEvents } from "../project-events";
import { ProjectRuntime } from "./project-runtime";
import { APIRQ } from "shared/api.requests";
import { joaatString } from "utils/joaat";
import { isAssetMetaFile, stripAssetMetaExt } from "utils/project";
import { ScopedLogService } from "backend/logger/scoped-logger";
import { lazyInject } from "backend/container-access";
import { ProjectApi } from "fxdk/project/common/project.api";
import { disposableTimeout } from "fxdk/base/async";

function hashFsWatcherEvent(event: FsWatcherEvent): string {
  return joaatString(`${event[0]}:${event[1]}:${event[2] ?? null}`);
}

enum ScanDepth {
  Deep,
  Auto,
  Shallow,
}

export class ProjectFsController implements IDisposableObject {
  @lazyInject(FsService)
  protected readonly fsService: FsService;

  protected readonly logService = new ScopedLogService('ProjectFS');

  @lazyInject(ApiClient)
  protected readonly apiClient: ApiClient;

  private path!: string;
  private rootFsEntry: IFsEntry | null = null;

  private toDispose = new Disposer();

  private fsEventsQueue: UniqueQueue<FsWatcherEvent>;

  private ignoredPaths: string[];

  private fsEventSuppression: Record<FsWatcherEventType, Set<string>> = {
    [FsWatcherEventType.CREATED]: new Set(),
    [FsWatcherEventType.DELETED]: new Set(),
    [FsWatcherEventType.MODIFIED]: new Set(),
    [FsWatcherEventType.RENAMED]: new Set(),
  };

  private fsUpdateErrorDisposals: Record<string, IDisposable> = {};

  constructor() {
    this.fsEventsQueue = this.toDispose.register(new UniqueQueue(this.processFsEvent, hashFsWatcherEvent));

    this.toDispose.register(this.apiClient.on(ProjectApi.FsEndpoints.shallowScanChildren, async (fsEntryPath) => {
      if (typeof fsEntryPath !== 'string') {
        return;
      }

      const fsEntry = this.getFsEntry(fsEntryPath);
      if (!fsEntry) {
        return;
      }

      await this.scanFsEntryChildren(fsEntryPath, fsEntry, ScanDepth.Shallow);

      this.sendFsUpdate({
        kind: FsUpdateKind.Update,
        fsEntry,
        path: this.getEntryPathParts(fsEntryPath),
      });
    }));
  }

  getFsEntry(fullEntryPath: string): IFsEntry | null {
    return this.getFsEntryByPath(this.getEntryPathParts(fullEntryPath));
  }

  getRootFsEntry(): IFsEntry {
    if (!this.rootFsEntry) {
      throw new Error('Missing root fs entry');
    }

    return this.rootFsEntry;
  }

  async init(rt: ProjectRuntime) {
    this.path = rt.state.path;

    this.ignoredPaths = [
      rt.state.storagePath,
      rt.state.manifestPath,
      this.fsService.joinPath(rt.state.path, '.vscode'),
    ];

    this.rootFsEntry = await this.spawnFsEntry(this.path);
    if (!this.rootFsEntry) {
      throw new Error('Failed to scan project fs entry');
    }

    this.toDispose.register(new FsWatcher({
      path: this.path,
      ignoredPaths: this.ignoredPaths,
      logger: (...args) => this.logService.log('[WATCHER]', ...args),
      onEvents: (events) => this.fsEventsQueue.appendMany(events),
    }));

    await this.scanFsEntryChildren(this.path, this.rootFsEntry);

    ProjectFsEvents.Hydrated.emit(this.rootFsEntry);
  }

  async dispose() {
    dispose(this.toDispose);

    for (const disposable of Object.values(this.fsUpdateErrorDisposals)) {
      dispose(disposable);
    }
  }

  //#region API
  async createFile({ filePath, fileName }: APIRQ.ProjectCreateFile): Promise<string> {
    const fileFullPath = this.fsService.joinPath(filePath, fileName);

    if (!(await this.fsService.statSafe(fileFullPath))) {
      await this.fsService.writeFile(fileFullPath, '');
    }

    return fileFullPath;
  }

  async createDirectory({ directoryName, directoryPath }: APIRQ.ProjectCreateDirectory) {
    const directoryFullPath = this.fsService.joinPath(directoryPath, directoryName);

    if (!(await this.fsService.statSafe(directoryFullPath))) {
      await this.fsService.mkdirp(directoryFullPath);
    }
  }

  async copyEntry(request: APIRQ.CopyEntry) {
    const sourceDirName = this.fsService.dirname(request.sourcePath);

    if (sourceDirName === request.targetPath) {
      return;
    }

    this.fsService.copy(request.sourcePath, request.targetPath);
  }

  async copyEntries(request: APIRQ.CopyEntries) {
    if (!request.sourcePaths.length) {
      return;
    }

    await Promise.all(request.sourcePaths.map((sourcePath) => {
      return this.fsService.copy(sourcePath, request.targetPath);
    }));
  }

  // This will trigger actual RENAMED fs event
  async renameEntry(request: APIRQ.RenameEntry) {
    const newEntryPath = this.fsService.joinPath(this.fsService.dirname(request.entryPath), request.newName);
    if (newEntryPath === request.entryPath) {
      return;
    }

    await ProjectFsEvents.BeforeRename.emit({ oldEntryPath: request.entryPath, entryPath: newEntryPath });

    await this.fsService.rename(request.entryPath, newEntryPath);

    this.logService.log('Renamed entry', request);
  }

  // While this one will trigger sequence of CREATED -> DELETED events, so we fake the renaming
  async moveEntry(request: APIRQ.MoveEntry) {
    const { sourcePath, targetPath } = request;

    const newPath = this.fsService.joinPath(targetPath, this.fsService.basename(sourcePath));
    if (newPath === sourcePath) {
      return;
    }

    await ProjectFsEvents.BeforeRename.emit({ oldEntryPath: sourcePath, entryPath: newPath });

    this.fsEventSuppression[FsWatcherEventType.CREATED].add(newPath);
    this.fsEventSuppression[FsWatcherEventType.DELETED].add(sourcePath);

    await this.fsService.rename(sourcePath, newPath);

    await this.processRenamed(newPath, sourcePath);
  }

  async deleteEntry(request: APIRQ.DeleteEntry): Promise<APIRQ.DeleteEntryResponse> {
    const { entryPath, hardDelete } = request;

    if (!(await this.fsService.statSafe(entryPath))) {
      return APIRQ.DeleteEntryResponse.Ok;
    }

    if (hardDelete) {
      await this.fsService.rimraf(entryPath);
    } else {
      try {
        await this.fsService.moveToTrashBin(entryPath);
      } catch (e) {
        this.logService.log(`Failed to recycle fs entry: ${e.toString()}`);
        this.fsEventsQueue.append(createFsWatcherEvent(FsWatcherEventType.MODIFIED, entryPath));
        return APIRQ.DeleteEntryResponse.FailedToRecycle;
      }
    }

    return APIRQ.DeleteEntryResponse.Ok;
  }

  async setEntryMeta(entryPath: string, meta: AssetMeta) {
    const entryMetaPath = this.getEntryMetaPath(entryPath);

    return this.fsService.writeFileJson(entryMetaPath, meta);
  }

  async deleteEntryMeta(entryPath: string) {
    const entryMetaPath = this.getEntryMetaPath(entryPath);

    return this.fsService.unlink(entryMetaPath);
  }
  //#endregion API

  private async spawnFsEntry(entryPath: string): Promise<IFsEntry | null> {
    const entryMetaPath = this.getEntryMetaPath(entryPath);

    let [stat, fxmeta] = await Promise.all([
      this.fsService.statSafeRetries(entryPath),
      this.fsService.readFileJson<AssetMeta>(entryMetaPath).catch(() => undefined),
    ]);

    if (!stat) {
      this.logService.error(new Error(`Failed to stat ${entryPath}`));
      return null;
    }

    const isDirectory = stat.isDirectory();

    const fsEntry: IFsEntry = {
      handle: isDirectory
        ? 'directory'
        : 'file',
      name: this.fsService.basename(entryPath),
      fxmeta,
      isDirectory,
      ctime: stat.ctimeMs,
      mtime: stat.mtimeMs,
      children: {},
      childrenScanned: false,
    };

    fsEntry.handle = await ProjectFsExtensions.getHandle(fsEntry, entryPath);

    await ProjectFsEvents.FsEntrySpawned.emit({ fsEntry, entryPath });

    return fsEntry;
  }

  private async updateFsEntry(entryPath: string, fsEntry: IFsEntry): Promise<Partial<IFsEntry> | null> {
    const mainStat = await this.fsService.statSafeRetries(entryPath);
    if (!mainStat) {
      // It could be that we have a delete event in the next events batch, so delay this error a bit
      this.addFsUpdateErrorNoEntry(entryPath);
      return null;
    }

    ([fsEntry.handle, fsEntry.fxmeta] = await Promise.all([
      ProjectFsExtensions.getHandle(fsEntry, entryPath),
      this.fsService.readFileJson<AssetMeta>(this.getEntryMetaPath(entryPath)).catch(() => undefined)
    ]));

    fsEntry.ctime = mainStat.ctimeMs;
    fsEntry.mtime = mainStat.mtimeMs;

    await ProjectFsEvents.FsEntryUpdated.emit({ fsEntry, entryPath });

    return {
      handle: fsEntry.handle,
      fxmeta: fsEntry.fxmeta,
      ctime: fsEntry.ctime,
      mtime: fsEntry.mtime,
    };
  }

  private addFsUpdateErrorNoEntry(entryPath: string) {
    this.extinguishFsUpdateError(entryPath);

    this.fsUpdateErrorDisposals[entryPath] = disposableTimeout(
      () => {
        this.logService.error(new Error(`Failed to update fs entry as it is missing in filesystem`), { entryPath });
      },
      1000, // defensively large timeout
    );
  }

  private extinguishFsUpdateError(entryPath: string) {
    if (this.fsUpdateErrorDisposals[entryPath]) {
      dispose(this.fsUpdateErrorDisposals[entryPath]);
      delete this.fsUpdateErrorDisposals[entryPath];
    }
  }

  private async scanFsEntryChildren(entryPath: string, fsEntry: IFsEntry, depth = ScanDepth.Auto) {
    const entryNames = await this.fsService.readdir(entryPath);

    await Promise.all(entryNames.map(async (entryName) => {
      // TODO: Git integration
      if (entryName === '.git') {
        return;
      }

      if (isAssetMetaFile(entryName)) {
        return;
      }

      const childEntryPath = this.fsService.joinPath(entryPath, entryName);
      if (this.ignoredPaths.find((ignoredPath) => childEntryPath.startsWith(ignoredPath))) {
        return;
      }

      const childFsEntry = await this.spawnFsEntry(childEntryPath);
      if (!childFsEntry) {
        return;
      }

      fsEntry.children[entryName] = childFsEntry;

      // So far only this folder needs lazy-loading
      // this implies no asset runtimes will be spawned for this folder children,
      // which would be weird in general
      if (childFsEntry.name === 'node_modules' && depth !== ScanDepth.Deep) {
        return;
      }

      if (childFsEntry.isDirectory && depth !== ScanDepth.Shallow) {
        await this.scanFsEntryChildren(childEntryPath, childFsEntry);
      }
    }));

    fsEntry.childrenScanned = true;

    await ProjectFsEvents.FsEntryChildrenScanned.emit({ fsEntry, entryPath });
  }

  private getEntryMetaPath(entryPath: string): string {
    const assetName = this.fsService.basename(entryPath);
    const assetMetaFilename = assetName + assetMetaFileExt;

    return this.fsService.joinPath(this.fsService.dirname(entryPath), assetMetaFilename);;
  }

  private readonly processFsEvent = async (event: FsWatcherEvent) => {
    const [type, entryPath, oldEntryPath] = event;

    const assetMetaFile = isAssetMetaFile(entryPath);

    if (!assetMetaFile) {
      // Expand events if needed
      this.fsEventsQueue.appendMany(ProjectFsExtensions.expandEvent(event));
    }

    // Handle suppression, yes, Set.delete returns whether or not it has deleted entry
    if (this.fsEventSuppression[type].delete(entryPath)) {
      return;
    }

    // Repurpose asset meta file update to it's associated entry MODIFIED event
    if (assetMetaFile) {
      this.fsEventsQueue.append(createFsWatcherEvent(FsWatcherEventType.MODIFIED, stripAssetMetaExt(entryPath)));

      return;
    }

    try {
      switch (type) {
        case FsWatcherEventType.CREATED: {
          await this.processCreated(entryPath);
          return;
        }
        case FsWatcherEventType.MODIFIED: {
          await this.processModified(entryPath);
          return;
        }
        case FsWatcherEventType.RENAMED: {
          if (oldEntryPath) {
            await this.processRenamed(entryPath, oldEntryPath);
          } else {
            await this.processCreated(entryPath);
          }
          return;
        }
        case FsWatcherEventType.DELETED: {
          await this.processDeleted(entryPath);
          return;
        }
      }
    } catch (e) {
      this.logService.error(e);
    }
  };

  private async processCreated(entryPath: string) {
    const fsEntry = await this.spawnFsEntry(entryPath);
    if (!fsEntry) {
      return;
    }

    const entryPathParts = this.getEntryPathParts(entryPath);
    const parentEntryPathParts = this.getEntryPathPartsParent(entryPathParts);

    // Only "legit" case when there's no parent fs entry - project root,
    // but that can't be created when we're already there
    const parentFsEntry = this.getFsEntryByPath(parentEntryPathParts);
    if (!parentFsEntry) {
      return;
    }

    parentFsEntry.children[fsEntry.name] = fsEntry;

    if (fsEntry.isDirectory && this.shouldScanFsEntryChildren(entryPath)) {
      await this.scanFsEntryChildren(entryPath, fsEntry);
    }

    this.sendFsUpdate({
      kind: FsUpdateKind.Set,
      parentPath: parentEntryPathParts,
      fsEntry,
    });

    await ProjectFsEvents.AfterCreated.emit({ fsEntry, entryPath });
  }

  private async processModified(entryPath: string) {
    const entryPathParts = this.getEntryPathParts(entryPath);

    const fsEntry = this.getFsEntryByPath(entryPathParts);
    if (!fsEntry) {
      return;
    }

    const update = await this.updateFsEntry(entryPath, fsEntry);
    if (!update) {
      return;
    }

    this.sendFsUpdate({
      kind: FsUpdateKind.Update,
      path: entryPathParts,
      fsEntry: update,
    });

    await ProjectFsEvents.AfterModified.emit({ fsEntry, entryPath });
  }

  private async processRenamed(entryPath: string, oldEntryPath: string) {
    const oldEntryPathParts = this.getEntryPathParts(oldEntryPath);
    const oldEntryName = this.getEntryPathPartsName(oldEntryPathParts);

    const fsEntry = this.getFsEntryByPath(oldEntryPathParts);
    if (!fsEntry) {
      this.logService.log(`Unable to handle rename event of entry that did not exist in project tree before`);
      return;
    }

    const newEntryPathParts = this.getEntryPathParts(entryPath);
    const newEntryName = this.getEntryPathPartsName(newEntryPathParts);

    const newParentEntryPathParts = this.getEntryPathPartsParent(newEntryPathParts);
    const newParentFsEntry = this.getFsEntryByPath(newParentEntryPathParts);

    const oldParentEntryPathParts = this.getEntryPathPartsParent(oldEntryPathParts);
    const oldParentFsEntry = this.getFsEntryByPath(oldParentEntryPathParts);

    delete oldParentFsEntry?.children[oldEntryName];

    if (newParentFsEntry) {
      newParentFsEntry.children[newEntryName] = fsEntry;
    }

    this.sendFsUpdate({
      kind: FsUpdateKind.Rename,
      oldName: oldEntryName,
      newName: newEntryName,
      newParentPath: newParentEntryPathParts,
      oldParentPath: oldParentEntryPathParts,
    });

    this.apiClient.emit(ProjectApi.FsEndpoints.entryRenamed, {
      fromEntryPath: oldEntryPath,
      toEntryPath: entryPath,
    });

    await ProjectFsEvents.AfterRenamed.emit({ fsEntry, oldEntryPath, entryPath });
  }

  private async processDeleted(oldEntryPath: string) {
    // There could be an update error pending
    this.extinguishFsUpdateError(oldEntryPath);

    const entryPathParts = this.getEntryPathParts(oldEntryPath);

    const parentEntryPathParts = this.getEntryPathPartsParent(entryPathParts);
    const entryName = this.getEntryPathPartsName(entryPathParts);

    const parentEntry = this.getFsEntryByPath(parentEntryPathParts);
    delete parentEntry?.children[entryName];

    this.sendFsUpdate({
      kind: FsUpdateKind.Delete,
      name: entryName,
      parentPath: parentEntryPathParts,
    });

    this.apiClient.emit(ProjectApi.FsEndpoints.entryDeleted, { entryPath: oldEntryPath });

    await ProjectFsEvents.AfterDeleted.emit({ oldEntryPath });
  }

  private shouldScanFsEntryChildren(entryPath: string): boolean {
    return true;
  }

  private getEntryPathParts(entryPath: string): string[] {
    return this.fsService.splitPath(entryPath.substr(this.path.length + 1));
  }

  private getEntryPathPartsParent(entryPathParts: string[]): string[] {
    return entryPathParts.slice(0, -1);
  }

  private getEntryPathPartsName(entryPathParts: string[]): string {
    return entryPathParts[entryPathParts.length - 1];
  }

  private getFsEntryByPath(entryPathParts: string[]): IFsEntry | null {
    return entryPathParts.reduce((fsEntry, pathPart) => fsEntry?.children[pathPart] || null, this.rootFsEntry);
  }

  private sendFsUpdate(update: IFsUpdate) {
    this.apiClient.emit(ProjectApi.FsEndpoints.update, update);
  }
}
