import { EntryMetaExtras, ExplorerService } from "backend/explorer/explorer-service";
import { inject, injectable } from "inversify";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { FsService } from "./fs-service";
import { ProjectFsUpdate } from 'shared/project.types';
import { LogService } from 'backend/logger/log-service';
import { FsWatcher, FsWatcherEvent, FsWatcherEventType } from './fs-watcher';
import { Queue } from "backend/queue";

export type FsMappingCreatedHandler = (entry: FilesystemEntry) => void | Promise<void>;
export type FsMappingDeletedHandler = (entryPath: string) => void | Promise<void>;
export type FsMappingModifiedHandler = (entry: FilesystemEntry) => void | Promise<void>;
export type FsMappingRenamedHandler = (entry: FilesystemEntry, oldEntryPath: string) => void | Promise<void>;

export type FsMappingProcessEntry = (entry: FilesystemEntry) => void | Promise<void>;
export type FsMappingShouldProcessUpdate = (path: string) => boolean;
export type FsMappingAfterUpdateHandler = (type: FsWatcherEventType, path: string, entry: FilesystemEntry | null) => void | Promise<void>;

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

  // protected watcher: chokidar.FSWatcher;
  protected watcher: FsWatcher;

  protected rootPath: string;
  protected ignoredPath: string = '';

  protected processEntry: FsMappingProcessEntry = () => {};
  setProcessEntry(fn: FsMappingProcessEntry) {
    this.processEntry = fn;
  }

  protected entryMetaExtras: EntryMetaExtras = {};
  setEntryMetaExtras(extras: EntryMetaExtras) {
    this.entryMetaExtras = extras;
  }

  protected onCreated: FsMappingCreatedHandler = () => {};
  setOnCreated(fn: FsMappingCreatedHandler) {
    this.onCreated = fn;
  }

  protected onDeleted: FsMappingDeletedHandler = () => {};
  setOnDeleted(fn: FsMappingDeletedHandler) {
    this.onDeleted = fn;
  }

  protected onModified: FsMappingModifiedHandler = () => {};
  setOnModified(fn: FsMappingModifiedHandler) {
    this.onModified = fn;
  }

  protected onRenamed: FsMappingRenamedHandler = () => {};
  setOnRenamed(fn: FsMappingRenamedHandler) {
    this.onRenamed = fn;
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

  protected queue: Queue<FsWatcherEvent>;

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

  async init(rootPath: string, ignored: string) {
    this.rootPath = rootPath;
    this.ignoredPath = ignored;

    this.queue = new Queue(([action, entryPath, oldEntryPath]) => this.processFsUpdate(action, entryPath, oldEntryPath));

    this.watcher = new FsWatcher({
      path: rootPath,
      ignoredPaths: [ignored],
      logger: (...args) => this.logService.log(...args),

      onEvent: (event) => this.queue.append(event),
    });

    this.map = await this.scanDir(rootPath);
  }

  async deinit() {
    this.queue.dispose();
    this.watcher.dispose();
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

        this.onModified(entry);
      }
    }
  }

  protected scanDir(path: string): Promise<FilesystemEntryMap> {
    return this.explorerService.readDirRecursively(
      path,
      this.entryMetaExtras,
      this.doProcessEntry,
    );
  }

  protected doProcessEntry = (entry: FilesystemEntry) => {
    if (!entry) {
      return;
    }

    if (this.ignoredPath && entry.path.indexOf(this.ignoredPath) > -1) {
      return;
    }

    return this.processEntry(entry);
  };

  protected async processFsUpdate(type: FsWatcherEventType, entryPath: string, oldEntryPath?: string) {
    if (!this.shouldProcessUpdate(entryPath)) {
      return;
    }

    const parentPath = this.fsService.dirname(entryPath);
    const entry = await this.explorerService.getEntry(entryPath, this.entryMetaExtras);

    // First apply changes
    const parent = this.map[parentPath];
    const promises = [];

    switch (type) {
      case FsWatcherEventType.CREATED: {
        if (!entry) {
          this.logService.error(new Error(`FS entry was created but stat has failed`), { entryPath, entry });
          return;
        }

        if (entry.isDirectory) {
          this.map[entryPath] = [];
          this.pendingUpdates.replace[entryPath] = [];
        }

        if (parent) {
          parent.push(entry);
          this.pendingUpdates.replace[parentPath] = parent;
        }

        this.onCreated(entry);
        promises.push(this.doProcessEntry(entry));

        break;
      }
      case FsWatcherEventType.DELETED: {
        this.logService.log('DELETED!', entryPath);

        if (this.map[entryPath]) {
          this.pendingUpdates.delete.push(entryPath);
          delete this.map[entryPath];
        }

        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === entryPath);

          if (updatedEntryIndex > -1) {
            parent.splice(updatedEntryIndex, 1);
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onDeleted(entryPath);

        break;
      }
      case FsWatcherEventType.MODIFIED: {
        // Modified emptiness?
        if (!entry) {
          this.logService.error(new Error(`FS entry was modified but stat has failed`), { entryPath, entry });
          return;
        }

        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === entryPath);

          if (updatedEntryIndex > -1) {
            parent[updatedEntryIndex] = entry;
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onModified(entry);
        promises.push(this.doProcessEntry(entry));

        break;
      }
      case FsWatcherEventType.RENAMED: {
        if (!entry) {
          this.logService.error(new Error(`FS entry was renamed but stat has failed`), { entryPath, entry, oldEntryPath });
          return;
        }

        // If that was a dir - scan it
        if (this.map[oldEntryPath]) {
          delete this.map[oldEntryPath];

          Object.assign(this.map, await this.scanDir(entryPath));

          this.pendingUpdates.delete.push(oldEntryPath);
          this.pendingUpdates.replace[entryPath] = this.map[entryPath];
        }

        if (parent) {
          const updatedEntryIndex = parent.findIndex((entry) => entry.path === oldEntryPath);

          if (updatedEntryIndex > -1) {
            parent[updatedEntryIndex] = entry;
            this.pendingUpdates.replace[parentPath] = parent;
          }
        }

        this.onRenamed(entry, oldEntryPath);
        promises.push(this.doProcessEntry(entry));

        break;
      }
    }

    await Promise.all(promises);

    this.afterUpdate(type, entryPath, entry);
  }
}
