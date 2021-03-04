import { inject, injectable } from "inversify";
import { URL } from 'url';
import http from 'http';
import https from 'https';
import yauzl from 'yauzl';
import { FsService } from "backend/fs/fs-service";
import { concurrently } from "utils/concurrently";

export interface ProgressReporter {
  setTotal(bytesLength: number): void;
  setDone(bytesLength: number): void;
}

const noopProgressReporter: ProgressReporter = {
  setTotal: () => {},
  setDone: () => {},
};

@injectable()
export class UpdaterUtils {
  @inject(FsService)
  protected readonly fsService: FsService;

  async downloadFile(targetPath: string, link: string, progressReporter: ProgressReporter = noopProgressReporter): Promise<void> {
    if (await this.fsService.statSafe(targetPath)) {
      throw new Error('Target path for file download already exist');
    }

    const linkUrl = new URL(link);

    let invokeTarget;
    if (linkUrl.protocol === 'http:') {
      invokeTarget =  http.get.bind(http, link);
    } else if (linkUrl.protocol === 'https:') {
      invokeTarget =  https.get.bind(http, link);
    } else {
      throw new Error('Unknown link protocol, must be either http or https');
    }

    return new Promise((resolve, reject) => {
      invokeTarget((res: http.IncomingMessage) => {
        let bytesWritten = 0;
        const bytesTotal = parseInt(res.headers['content-length'] || '0', 10) || 0;

        const targetWriteStream = this.fsService.createWriteStream(targetPath);

        progressReporter.setTotal(bytesTotal);

        res.pipe(targetWriteStream)

        targetWriteStream.on('finish', () => {
          progressReporter.setDone(bytesTotal);
          resolve();
        });

        res.on('data', (data) => {
          bytesWritten += data.length;

          progressReporter.setDone(bytesWritten);
        });
      }).on('error', (e) => reject(e));;
    });
  }

  async unpackZipFile(zipFilePath: string, targetPath: string, progressReporter: ProgressReporter = noopProgressReporter): Promise<void> {
    const [zipFilePathStat, targetPathStat] = await concurrently(
      this.fsService.statSafe(zipFilePath),
      this.fsService.statSafe(targetPath),
    );

    if (targetPathStat) {
      throw new Error('Target path for zip unpack already exist');
    }

    if (!zipFilePathStat) {
      throw new Error('No zip file');
    }

    let bytesUnpacked = 0;
    progressReporter.setTotal(zipFilePathStat.size);

    await this.fsService.mkdirp(targetPath);

    return new Promise((resolve, reject) => {
      yauzl.open(zipFilePath, { lazyEntries: true }, (err, zipFile) => {
        if (err) {
          return reject(err);
        }

        zipFile.readEntry();

        const createdFolders = new Set();

        zipFile.on('entry', (entry) => {
          const entryPath = this.fsService.joinPath(targetPath, entry.fileName);

          if (entry.fileName.endsWith('/')) {
            this.fsService.mkdirpSync(entryPath);
            createdFolders.add(entryPath);

            zipFile.readEntry();
          } else {
            const entryPathDirName = this.fsService.dirname(entryPath);

            if (!createdFolders.has(entryPathDirName)) {
              this.fsService.mkdirpSync(entryPathDirName);
              createdFolders.add(entryPathDirName);
            }

            zipFile.openReadStream(entry, (err, readStream) => {
              if (err) {
                return reject(err);
              }

              bytesUnpacked += entry.compressedSize;

              progressReporter.setDone(bytesUnpacked);

              readStream.on('end', () => zipFile.readEntry());
              readStream.pipe(this.fsService.createWriteStream(entryPath));
            });
          }
        });

        zipFile.on('close', () => {
          progressReporter.setTotal(zipFilePathStat.size);
          resolve();
        });
      });
    });
  }
}
