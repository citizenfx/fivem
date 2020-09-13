import * as fs from 'fs';
import * as path from 'path';
import * as mkdirp from 'mkdirp';
import * as cp from 'child_process';

import { ApiClient, ExplorerChildsMap, FilesystemEntry, FilesystemEntryMeta } from "./api.types";
import { explorerApi } from "./events";
import { fxdkProjectFilename, resourceManifestFilename, resourceManifestLegacyFilename } from './constants';
import { notNull } from './utils';

const dirComparator = (a: FilesystemEntry, b: FilesystemEntry): number => {
  if (a.isDirectory && !b.isDirectory) {
    return -1;
  }

  if (!a.isDirectory && b.isDirectory) {
    return 1;
  }

  return 0;
};

export type EntryMetaExtras = {
  [key: string]: (entryPath: string) => Promise<any>,
};

export class ExplorerApi {
  private drives: string[];

  constructor(
    private readonly client: ApiClient,
  ) {
    this.client.on(explorerApi.readRoots, this.onReadRoots);
    this.client.on(explorerApi.readRoot, this.onReadRoot);
    this.client.on(explorerApi.readDir, this.onReadDir);
    this.client.on(explorerApi.readDirRecursive, this.onReadDirRecursive);
    this.client.on(explorerApi.createDir, this.onCreateDir);
  }

  onReadRoots = async () => {
    const drives = await this.readDrives();

    const roots: FilesystemEntry[] = [];

    for (const drive of drives) {
      roots.push({
        path: drive,
        name: drive.substr(0, 2),
        meta: {},
        isFile: false,
        isDirectory: true,
        isSymbolicLink: false,
        children: await this.readDir(drive),
      });
    }

    this.client.emit(explorerApi.roots, roots);
  };

  onReadRoot = async (dir: string) => {
    const entry = await this.getEntry(dir);

    this.client.emit(explorerApi.root, entry);
  };

  onReadDir = async (dir: string) => {
    const children = await this.readDir(dir);

    this.client.emit(explorerApi.dir, {
      dir,
      children,
    });
  };

  onReadDirRecursive = async (dir: string) => {
    const pathsMap = await this.readDirRecursively(dir);

    this.client.emit(explorerApi.dirRecursive, pathsMap);
  };

  onCreateDir = async (dir: string) => {
    await this.createDir(dir);
  };

  async createDir(dir: string): Promise<void> {
    try {
      await fs.promises.stat(dir);
    } catch (e) {
      await mkdirp(dir);
    } finally {
      return this.client.emit(explorerApi.newDir, dir);
    }
  }

  async readDrives() {
    if (!this.drives) {
      this.drives = await new Promise((resolve) => {
        cp.exec('wmic logicaldisk get name', { windowsHide: true }, (error, stdout) => {
          resolve(stdout.split('\n')
            .filter(value => /[A-Za-z]:/.test(value))
            .map(value => value.trim() + '/'));
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
