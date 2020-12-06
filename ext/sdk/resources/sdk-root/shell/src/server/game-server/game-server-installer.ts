import http from 'https';
import yauzl from 'yauzl';
import { inject, injectable } from 'inversify';
import { FsService } from 'server/fs/fs-service';

export const versionFilename = '.fxserver-version';

export type Setter = (size: number) => void;

@injectable()
export class GameServerInstaller {
  @inject(FsService)
  protected readonly fsService: FsService;

  async downloadArtifact(url: string, artifactPath: string, setContentLength: Setter, setDataLength: Setter) {
    if (await this.fsService.statSafe(artifactPath)) {
      await this.fsService.unlink(artifactPath);
    }

    return new Promise((resolve, reject) => {
      http.get(url, (res) => {
        const artifactWriteStream = this.fsService.createWriteStream(artifactPath);

        const contentLength = parseInt(res.headers['content-length'] || '0', 10) || 0;
        setContentLength(contentLength);

        res.pipe(artifactWriteStream)

        artifactWriteStream.on('finish', () => {
          console.log('Finish writing artifact', artifactPath);
          artifactWriteStream.close();
          resolve(artifactPath);
        });

        res.on('data', (data) => setDataLength(data.length));
        res.on('end', () => {
          console.log('Finish downloading artifact', url);
        });
      }).on('error', (e) => reject(e));
    });
  }

  async unpackArtifact(artifactPath: string, artifactExtractionPath: string, setContentLength: Setter, setDataLength: Setter) {
    if (await this.fsService.statSafe(artifactExtractionPath)) {
      await this.fsService.unlink(artifactExtractionPath);
    }

    await this.fsService.mkdirp(artifactExtractionPath);

    return new Promise<void>((resolveUnpack, rejectUnpack) => {
      const p = (resolve, reject) => {
        let tries = 0;

        const tryStat = () => {
          tries++;

          try {
            setContentLength(this.fsService.statSync(artifactPath).size);
            console.log('Artifact size acquired');
            return resolve();
          } catch (e) {
            if (tries > 5) {
              return reject(e);
            }

            setTimeout(tryStat, 100);
          }
        };

        tryStat();
      };

      (new Promise<void>(p))
        .then(() => new Promise<void>(() => {
          yauzl.open(artifactPath, { lazyEntries: true }, (err, zipFile) => {
            if (err) {
              return rejectUnpack(err);
            }

            zipFile.readEntry();
            zipFile.on('entry', (entry) => {
              const entryPath = this.fsService.joinPath(artifactExtractionPath, entry.fileName);

              if (entry.fileName.endsWith('/')) {
                this.fsService.mkdirpSync(entryPath);
                zipFile.readEntry();
              } else {
                zipFile.openReadStream(entry, (err, readStream) => {
                  if (err) {
                    return rejectUnpack(err);
                  }

                  setDataLength(entry.compressedSize);

                  readStream.on('end', () => zipFile.readEntry());
                  readStream.pipe(this.fsService.createWriteStream(entryPath));
                });
              }
            });
            zipFile.on('close', () => resolveUnpack());
          });
        }))
    });
  }

  async prepareServer(artifactExtractionPath: string, version: string) {
    const componentsPath = this.fsService.joinPath(artifactExtractionPath, 'components.json');
    const componentsContent = await this.fsService.readFile(componentsPath);

    const components = JSON.parse(componentsContent.toString('utf8'));
    const newComponents = components.filter((component) => component !== 'svadhesive');

    await this.fsService.writeFile(componentsPath, JSON.stringify(newComponents, null, 2));

    // Sequential to make sure it will be added in the very end
    // because installer relies on this file
    await this.fsService.writeFile(this.fsService.joinPath(artifactExtractionPath, versionFilename), version);
  }
}
