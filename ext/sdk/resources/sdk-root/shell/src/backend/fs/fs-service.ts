import fs from 'fs';
import os from 'os';
import path from 'path';
import mkdirp from 'mkdirp';
import rimrafSync from 'rimraf';
import { promisify } from 'util';
import { injectable } from "inversify";
import { FsAtomicWriter } from './fs-atomic-writer';
import { FsJsonFileMapping, FsJsonFileMappingOptions } from './fs-json-file-mapping';

const rimraf = promisify(rimrafSync);

/**
 * Yes, stupid-ass abstraction
 */
@injectable()
export class FsService {
  tmpdir() {
    return os.tmpdir();
  }

  basename(entryPath: string) {
    return path.basename(entryPath);
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

  mkdirp(entryPath: string) {
    return mkdirp(entryPath);
  }

  mkdirpSync(entryPath: string) {
    return mkdirp.sync(entryPath);
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

  writeFile(entryPath: string, content: string | Buffer) {
    return fs.promises.writeFile(entryPath, content);
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
}
