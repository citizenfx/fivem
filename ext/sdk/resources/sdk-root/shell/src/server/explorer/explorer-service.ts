import fs from 'fs';
import path from 'path';
import mkdirp from 'mkdirp';
import cp from 'child_process';
import { inject, injectable } from 'inversify';
import { ApiClient } from '../api/api-client';
import { LogService } from 'server/logger/log-service';
import { FsService } from 'server/fs/fs-service';
import { handlesClientEvent } from '../api/api-decorators';
import { explorerApi } from 'shared/api.events';
import { ExplorerChildsMap, FilesystemEntry, FilesystemEntryMeta } from 'shared/api.types';
import { notNull } from 'shared/utils';
import { fxdkProjectFilename, resourceManifestFilename, resourceManifestLegacyFilename } from '../constants';
import { NotificationService } from 'server/notification/notification-service';
import { ApiContribution } from 'server/api/api-contribution';

export type EntryMetaExtras = {
  [key: string]: (entryPath: string) => Promise<any>,
};

export const dirComparator = (a: FilesystemEntry, b: FilesystemEntry): number => {
  if (a.isDirectory && !b.isDirectory) {
    return -1;
  }

  if (!a.isDirectory && b.isDirectory) {
    return 1;
  }

  return 0;
};

@injectable()
export class ExplorerService implements ApiContribution {
  getId() {
    return 'ExplorerService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private drives: string[] = [];
  private drivesInitialized = false;

  @handlesClientEvent(explorerApi.readRoots)
  async readRoots() {
    const drives = await this.readDrives();

    const roots: FilesystemEntry[] = (await Promise.all(
      drives.map(async (drive) => {
        let children;
        try {
          children = await this.readDir(drive + '/');
        } catch (e) {
          this.notificationService.warning(`Failed to read drive ${drive}: ${e.toString()}`);

          return null;
        }

        return {
          path: drive,
          name: drive,
          meta: {},
          isFile: false,
          isDirectory: true,
          isSymbolicLink: false,
          children,
        };
      }),
    )).filter(notNull);

    this.apiClient.emit(explorerApi.roots, roots);
  }

  @handlesClientEvent(explorerApi.readRoot)
  async readRoot(dir: string) {
    const entry = await this.getEntry(dir);

    this.apiClient.emit(explorerApi.root, entry);
  };

  @handlesClientEvent(explorerApi.readDir)
  async readDirToClient(dir: string) {
    const children = await this.readDir(dir);

    this.apiClient.emit(explorerApi.dir, {
      dir,
      children,
    });
  };

  @handlesClientEvent(explorerApi.readDirRecursive)
  async onReadDirRecursive(dir: string) {
    const pathsMap = await this.readDirRecursively(dir);

    this.apiClient.emit(explorerApi.dirRecursive, pathsMap);
  };

  @handlesClientEvent(explorerApi.createDir)
  async createDir(dir: string): Promise<void> {
    try {
      await fs.promises.stat(dir);
    } catch (e) {
      await mkdirp(dir);
    } finally {
      return this.apiClient.emit(explorerApi.newDir, dir);
    }
  }

  async readDrives() {
    if (!this.drivesInitialized) {
      this.drivesInitialized = true;

      this.drives = await new Promise((resolve) => {
        cp.exec('wmic logicaldisk get name', { windowsHide: true }, (error, stdout) => {
          resolve(stdout.split('\n')
            .filter(value => /[A-Za-z]:/.test(value))
            .map(value => value.trim()));
        });
      });
    }

    return this.drives;
  }

  async isFxdkProject(dir: string): Promise<boolean> {
    const manifestPath = path.join(dir, fxdkProjectFilename);

    try {
      await fs.promises.stat(manifestPath);

      return true;
    } catch (e) {
      return false;
    }
  }

  async isResource(dir: string): Promise<boolean> {
    const resourceManifestPath = path.join(dir, resourceManifestFilename);
    const oldResourceManifestPath = path.join(dir, resourceManifestLegacyFilename);

    try {
      await fs.promises.stat(resourceManifestPath);

      return true;
    } catch (e) {
      // welp, let's try old one then
    }

    try {
      await fs.promises.stat(oldResourceManifestPath);

      return true;
    } catch (e) {
      return false;
    }
  }

  async getEntry(entryPath: string, extras?: EntryMetaExtras): Promise<FilesystemEntry | null> {
    try {
      const stats = await fs.promises.stat(entryPath);

      const isFile = stats.isFile();
      const isDirectory = stats.isDirectory();
      const isSymbolicLink = stats.isSymbolicLink();
      const meta: FilesystemEntryMeta = {};

      if (isDirectory) {
        meta.isFxdkProject = await this.isFxdkProject(entryPath);
        meta.isResource = await this.isResource(entryPath);

        if (extras) {
          await Promise.all([
            Object.entries(extras).map(async ([key, extraFetcher]) => {
              meta[key] = await extraFetcher(entryPath);
            }),
          ]);
        }
      }

      return {
        path: entryPath,
        name: path.basename(entryPath),
        meta,
        isFile,
        isDirectory,
        isSymbolicLink,
      };
    } catch (e) {
      return null;
    }
  }

  async readDir(dir: string, extras?: EntryMetaExtras): Promise<FilesystemEntry[]> {
    const entries = await fs.promises.readdir(dir);
    const entriesStats = await Promise.all(
      entries.map((file) => this.getEntry(path.join(dir, file), extras)),
    );

    return entriesStats
      .filter(notNull)
      .sort(dirComparator);
  }

  async readDirRecursively(dir: string, extras?: EntryMetaExtras): Promise<ExplorerChildsMap> {
    const entries = await this.readDir(dir, extras);

    let cache = {
      [dir]: entries,
    };

    await Promise.all(
      entries.map(async (entry) => {
        if (entry.isDirectory) {
          Object.assign(cache, await this.readDirRecursively(entry.path, extras));
        }
      }),
    );

    return cache;
  }
}
