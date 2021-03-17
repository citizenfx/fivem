import chokidar from 'chokidar';
import { EntryMetaExtras, ExplorerService } from "backend/explorer/explorer-service";
import { inject, injectable } from "inversify";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { FsService } from "./fs-service";
import { ProjectFsUpdate } from 'shared/project.types';
import { LogService } from 'backend/logger/log-service';


export enum FsUpdateType {
  add,
  addDir,
  change,
  unlink,
  unlinkDir,
}

export type FsMappingProcessEntry = (entry: FilesystemEntry) => void | Promise<void>;
export type FsMappingAddOrChangeHandler = (entry: FilesystemEntry) => void | Promise<void>;
export type FsMappingUnlinkHandler = (entryPath: string) => void | Promise<void>;
export type FsMappingShouldProcessUpdate = (type: FsUpdateType, path: string) => boolean;
export type FsMappingAfterUpdateHandler = (type: FsUpdateType, path: string, entry: FilesystemEntry | null) => void | Promise<void>;

@injectable()
export class FsMapping {
  @inject(ExplorerService)
  protected readonly explorerService: ExplorerService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  protected map: FilesystemEntryMap = {};
  getMap(): FilesystemEntryMap {
    return this.map;
  }

  protected watcher: chokidar.FSWatcher;
  protected rootPath: string;

  protected processEntry: FsMappingProcessEntry = () => {};
  setProcessEntry(fn: FsMappingProcessEntry) {
    this.processEntry = fn;
  }

  protected entryMetaExtras: EntryMetaExtras = {};
  setEntryMetaExtras(extras: EntryMetaExtras) {
    this.entryMetaExtras = extras;
  }

  protected onAdd: FsMappingAddOrChangeHandler = () => {};
  setOnAdd(fn: FsMappingAddOrChangeHandler) {
    this.onAdd = fn;
  }

  protected onAddDir: FsMappingAddOrChangeHandler = () => {};
  setOnAddDir(fn: FsMappingAddOrChangeHandler) {
    this.onAddDir = fn;
  }

  protected onChange: FsMappingAddOrChangeHandler = () => {};
  setOnChange(fn: FsMappingAddOrChangeHandler) {
    this.onChange = fn;
  }

  protected onUnlink: FsMappingUnlinkHandler = () => {};
  setOnUnlink(fn: FsMappingUnlinkHandler) {
    this.onUnlink = fn;
  }

  protected onUnlinkDir: FsMappingUnlinkHandler = () => {};
  setOnUnlinkDir(fn: FsMappingUnlinkHandler) {
    this.onUnlinkDir = fn;
  }

  protected shouldProcessUpdate: FsMappingShouldProcessUpdate = () => true;
  setShouldProcessUpdate(fn: FsMappingShouldProcessUpdate) {
    this.shouldProcessUpdate = fn;
  }

  protected afterUpdate: FsMappingAfterUpdateHandler = () => {};
  setAfterUpdate(fn: FsMappingAfterUpdateHandler) {
    this.afterUpdate = fn;
  }

  protected pendingUpdates: ProjectFsUpdate = {
    delete: [],
    replace: {},
  };

  hasUpdates(): boolean {
    return this.pendingUpdates.delete.length > 0 && Object.keys(this.pendingUpdates.replace).length > 0;
  }

  flushUpdates(): ProjectFsUpdate {
    const update = this.pendingUpdates;
    this.pendingUpdates = {
      delete: [],
      replace: {},
    };

    return update;
  }

  async init(rootPath: string, ignored: any) {
    this.rootPath = rootPath;

    this.watcher = chokidar.watch(rootPath, {
      ignored,
      persistent: true,
      ignoreInitial: true,
      disableGlobbing: true,
      ignorePermissionErrors: true,
    });

    this.watcher
      .on('add', (updatedPath: string) => this.processFsUpdate(FsUpdateType.add, updatedPath))
      .on('addDir', (updatedPath: string) => this.processFsUpdate(FsUpdateType.addDir, updatedPath))
      .on('change', (updatedPath: string) => this.processFsUpdate(FsUpdateType.change, updatedPath))
      .on('unlink', (updatedPath: string) => this.processFsUpdate(FsUpdateType.unlink, updatedPath))
      .on('unlinkDir', (updatedPath: string) => this.processFsUpdate(FsUpdateType.unlinkDir, updatedPath));

    this.map = await this.explorerService.readDirRecursively(
      rootPath,
      this.entryMetaExtras,
      this.processEntry,
    );
  }

  async deinit() {
    await this.watcher.close();
  }

  async forceEntryScan(entryPath: string) {
    const entry = await this.explorerService.getEntry(entryPath, this.entryMetaExtras);

    const parentPath = this.fsService.dirname(entryPath);
    const parent = this.map[parentPath];

    if (entry && parent) {
      const entryIndexInParent = parent.findIndex((parentEntry) => parentEntry.path === entryPath);

      if (entryIndexInParent > -1) {
        parent[entryIndexInParent] = entry;
        this.pendingUpdates.replace[parentPath] = parent;

        this.onChange(entry);
      }
    }
  }

  protected async processFsUpdate(type: FsUpdateType, path: string) {
    this.logService.log('AAA', { type: FsUpdateType[type], path });

    if (!this.shouldProcessUpdate(type, path)) {
      return;
    }

    const parentPath = this.fsService.dirname(path);
    const entry = await this.explorerService.getEntry(path, this.entryMetaExtras);

    // First apply changes
    const parent = this.map[parentPath];
    const promises = [];

    switch (type) {
      case FsUpdateType.addDir: {
        if (!entry) {
          break;
        }

        this.map[path] = [];
        this.pendingUpdates.replace[path] = [];

        if (parent) {
          parent.push(entry);
          this.pendingUpdates.replace[parentPath] = parent;
        }

        this.onAddDir(entry);
        promises.push(this.processEntry(entry));

        break;
      }
      case FsUpdateType.add: {
        if (!entry) {
          break;
        }

        if (parent) {
          parent.push(entry);
          this.pendingUpdates.replace[parentPath] = parent;
        }

        this.onAdd(entry);
        promises.push(this.processEntry(entry));

        break;
      }

      case FsUpdateType.change: {
        if (!entry) {
          break;
        }

        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === path);

          if (updatedEntryIndex > -1) {
            parent[updatedEntryIndex] = entry;
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onChange(entry);
        promises.push(this.processEntry(entry));

        break;
      }

      case FsUpdateType.unlinkDir: {
        this.pendingUpdates.delete.push(path);
        delete this.map[path];

        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === path);

          if (updatedEntryIndex > -1) {
            parent.splice(updatedEntryIndex, 1);
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onUnlinkDir(path);

        break;
      }
      case FsUpdateType.unlink: {
        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === path);

          if (updatedEntryIndex > -1) {
            parent.splice(updatedEntryIndex, 1);
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onUnlink(path);

        break;
      }
    }

    await Promise.all(promises);

    this.afterUpdate(type, path, entry);
  }
}
