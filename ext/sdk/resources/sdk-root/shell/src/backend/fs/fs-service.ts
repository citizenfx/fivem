import fs from 'fs';
import os from 'os';
import path from 'path';
import fg from 'fast-glob';
import mkdirp from 'mkdirp';
import rimrafSync from 'rimraf';
import { promisify } from 'util';
import { inject, injectable } from "inversify";
import { FsAtomicWriter } from './fs-atomic-writer';
import { FsJsonFileMapping, FsJsonFileMappingOptions } from './fs-json-file-mapping';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { concurrently } from 'utils/concurrently';
import { fastRandomId } from 'utils/random';

const rimraf = promisify(rimrafSync);

export interface CopyOptions {
  onProgress?: (progress: number) => void,
}

/**
 * Yes, stupid-ass abstraction
 */
@injectable()
export class FsService {
  @inject(LogService)
  protected readonly logService: LogService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  tmpdir() {
    return os.tmpdir();
  }

  basename(entryPath: string, ext?: string) {
    return path.basename(entryPath, ext);
  }

  dirname(entryPath: string) {
    return path.dirname(entryPath);
  }

  isAbsolutePath(entryPath: string) {
    return path.isAbsolute(entryPath);
  }

  relativePath(from: string, to: string) {
    return path.relative(from, to);
  }

  joinPath(...args: string[]) {
    return path.join(...args);
  }

  resolvePath(entryPath: string) {
    return path.resolve(entryPath);
  }

  async statSafe(entryPath: string): Promise<fs.Stats | null> {
    try {
      return await this.stat(entryPath);
    } catch (e) {
      return null;
    }
  }

  stat(entryPath: string) {
    return fs.promises.stat(entryPath);
  }

  statSync(entryPath: string) {
    return fs.statSync(entryPath);
  }

  rimraf(entryPath: string) {
    return rimraf(entryPath);
  }

  async ensureDeleted(entryPath: string) {
    const stat = await this.statSafe(entryPath);

    if (stat) {
      if (stat.isDirectory()) {
        await this.rimraf(entryPath);
      } else {
        await this.unlink(entryPath);
      }
    }
  }

  readdir(entryPath: string): Promise<string[]> {
    return fs.promises.readdir(entryPath);
  }

  mkdirp(entryPath: string) {
    return mkdirp(entryPath);
  }

  mkdirpSync(entryPath: string) {
    return mkdirp.sync(entryPath);
  }

  async ensureDir(entryPath: string) {
    const stat = await this.statSafe(entryPath);

    if (!stat) {
      return this.mkdirp(entryPath);
    }

    if (!stat.isDirectory()) {
      throw new Error(`${entryPath} exist but is not a directory`);
    }
  }

  unlink(entryPath: string) {
    return fs.promises.unlink(entryPath);
  }

  rename(entryPath: string, newEntryPath: string) {
    return fs.promises.rename(entryPath, newEntryPath);
  }

  createReadStream(entryPath: string) {
    return fs.createReadStream(entryPath);
  }

  createWriteStream(entryPath: string) {
    return fs.createWriteStream(entryPath);
  }

  readFile(entryPath: string) {
    return fs.promises.readFile(entryPath);
  }

  async readFileString(entryPath: string): Promise<string> {
    return (await this.readFile(entryPath)).toString();
  }

  async readFileJson<T>(entryPath: string): Promise<T> {
    const content = await this.readFileString(entryPath);

    return JSON.parse(content);
  }

  writeFile(entryPath: string, content: string | Buffer) {
    return fs.promises.writeFile(entryPath, content);
  }

  writeFileJson<T>(entryPath: string, content: T, pretty = true) {
    return this.writeFile(entryPath, JSON.stringify(content, undefined, pretty ? 2 : undefined));
  }

  createAtomicWrite(entryPath: string) {
    return new FsAtomicWriter(entryPath, this.writeFile.bind(this));
  }

  async createJsonFileMapping<T extends object>(
    entryPath: string,
    options?: FsJsonFileMappingOptions<T>,
  ): Promise<FsJsonFileMapping<T>> {
    const writer = this.createAtomicWrite(entryPath);
    const reader = async () => {
      const content = await this.readFile(entryPath);
      return {
        ...(options?.defaults || null),
        ...JSON.parse(content.toString()),
      };
    };

    const snapshot = await reader();

    return new FsJsonFileMapping(snapshot, writer, reader, options);
  }

  /**
   * If sourcePath is /a/b/entry
   * And target is /a/c
   * It will copy content of /a/b/entry to /a/c/entry
   *
   * @param sourcePath
   * @param target
   */
  async copy(sourcePath: string, target: string, options: CopyOptions = {}) {
    const sourceName = this.basename(sourcePath);
    const targetPath = this.joinPath(target, sourceName);

    const sourceStat = await this.statSafe(sourcePath);
    if (!sourceStat) {
      throw new Error(`Can't copy ${sourcePath} -> ${target} as ${sourcePath} does not exist`);
    }

    const {
      onProgress = () => {},
    } = options;

    // Handle file first
    if (!sourceStat.isDirectory()) {
      const sourceSize = sourceStat.size;
      let doneSize = 0;

      if (onProgress) {
        return this.doCopyFile(sourcePath, targetPath, onProgress);
      }

      return this.taskReporterService.wrap(`Copying ${sourcePath} to ${target}`, (task) => {
        return this.doCopyFile(sourcePath, targetPath, (bytesRead) => {
          doneSize += bytesRead;
          task.setProgress(doneSize / sourceSize);
        });
      });
    }

    // Now directories
    if (onProgress) {
      return this.doCopyDir(sourcePath, targetPath, onProgress);
    }

    return this.taskReporterService.wrap(`Copying ${sourcePath} to ${target}`, async (task) => {
      return this.doCopyDir(sourcePath, targetPath, (progress: number) => task.setProgress(progress));
    });
  }

  async copyFileContent(sourcePath: string, targetPath: string, options: CopyOptions = {}): Promise<void> {
    const sourcePathStats = await this.statSafe(sourcePath);
    if (!sourcePathStats) {
      throw new Error('No source path: '+ sourcePath);
    }

    return new Promise((resolve, reject) => {
      const reader = this.createReadStream(sourcePath);
      const writer = this.createWriteStream(targetPath);

      let finished = false;
      const finish = (error?: Error) => {
        if (!finished) {
          finished = true;

          if (error) {
            return reject(error);
          }

          return resolve();
        }
      };

      reader.once('error', finish);
      writer.once('error', finish);

      writer.once('close', () => finish());

      if (options.onProgress) {
        const totalSize = sourcePathStats.size;
        let doneSize = 0;

        reader.on('data', (chunk) => {
          doneSize += chunk.length;

          options.onProgress(doneSize / totalSize);
        });
      }

      reader.pipe(writer);
    });
  }

  /**
   * Copies /a/b/* to /a/c/*
   */
  async copyDirContent(sourcePath: string, targetPath: string, options: CopyOptions = {}) {
    const {
      onProgress = () => {},
    } = options;

    return this.doCopyDir(sourcePath, targetPath, onProgress);
  }

  private async doCopyDir(
    sourcePath: string,
    targetPath: string,
    reportProgress: (bytesRead: number) => void = () => {},
  ): Promise<void> {
    type SourcePath = string;
    type TargetPath = string;

    const sourceDirPaths = new Set<SourcePath>([sourcePath]);
    const filesToCopy = new Map<SourcePath, TargetPath>();
    const ignoredFiles = new Set();

    let totalSize = 0;
    let doneSize = 0;

    for (const sourceDirPath of sourceDirPaths) {
      const targetDirPath = this.joinPath(targetPath, this.relativePath(sourcePath, sourceDirPath));

      const [dirEntryNames] = await concurrently(
        this.readdir(sourceDirPath),
        this.mkdirp(targetDirPath),
      );

      await Promise.all(dirEntryNames.map(async (dirEntryName) => {
        const dirEntrySourcePath = this.joinPath(sourceDirPath, dirEntryName);
        const dirEntryTargetPath = this.joinPath(targetDirPath, dirEntryName);

        const dirEntrySourceStat = await this.statSafe(dirEntrySourcePath);
        if (dirEntrySourceStat) {
          if (dirEntrySourceStat.isDirectory()) {
            return sourceDirPaths.add(dirEntrySourcePath);
          }

          // TODO: Support file overwrites
          if (await this.statSafe(dirEntryTargetPath)) {
            return ignoredFiles.add(dirEntrySourcePath);
          }

          filesToCopy.set(dirEntrySourcePath, dirEntryTargetPath);
          totalSize += dirEntrySourceStat.size;
        }
      }));
    }

    if (ignoredFiles.size) {
      this.notificationService.warning(`Following files won't be copied as they will overwrite existing files: ${[...ignoredFiles.values()].join(', ')}`);
    }

    for (const [fileSourcePath, fileTargetPath] of filesToCopy.entries()) {
      await this.doCopyFile(fileSourcePath, fileTargetPath, (bytesRead) => {
        doneSize += bytesRead;
        reportProgress(doneSize / totalSize);
      });
    }
  }

  private doCopyFile(sourcePath: string, targetPath: string, reportProgress: (bytesRead: number) => void = () => {}): Promise<void> {
    return new Promise((resolve, reject) => {
      const reader = this.createReadStream(sourcePath);
      const writer = this.createWriteStream(targetPath);

      let finished = false;
      const finish = (error?: Error) => {
        if (!finished) {
          finished = true;

          if (error) {
            return reject(error);
          }

          return resolve();
        }
      };

      reader.once('error', finish);
      writer.once('error', finish);

      writer.once('close', () => finish());

      reader.on('data', (chunk) => reportProgress(chunk.length));

      reader.pipe(writer);
    });
  }

  async glob(patterns: string | string[], options: fg.Options): Promise<string[]> {
    return fg(patterns, options);
  }

  async readIgnorePatterns(basePath: string, fileName: string = '.fxdkignore'): Promise<string[]> {
    try {
      const filePath = this.joinPath(basePath, fileName);
      const fileContent = await this.readFileString(filePath);

      return fileContent.trim().split('\n').map((line) => line.trim()).filter(Boolean);
    } catch (e) {
      return [];
    }
  }

  async moveToTrashBin(...entryPaths: string[]): Promise<void> {
    await new Promise<void>((resolve, reject) => {
      const msgId = fastRandomId();

      emit('sdk:recycleShellItems', msgId, entryPaths);

      const handler = (rmsgId: string, error: string) => {
        if (msgId !== rmsgId) {
          return;
        }

        RemoveEventHandler('sdk:recycleShellItemsResponse', handler);

        if (error) {
          return reject(new Error(error));
        }

        return resolve();
      }

      on('sdk:recycleShellItemsResponse', handler);
    });
  }
}
