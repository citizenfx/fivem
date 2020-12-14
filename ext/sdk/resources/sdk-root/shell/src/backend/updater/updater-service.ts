import tar from 'tar-fs';
import { inject, injectable } from "inversify";
import { StatusService } from "backend/status/status-service";
import { AppContribution } from "backend/app/app-contribution";
import { ConfigService } from "backend/config-service";
import { Deferred } from "backend/deferred";
import { FsService } from "backend/fs/fs-service";
import { updaterStatuses } from "shared/api.statuses";
import { fastRandomId } from 'utils/random';

@injectable()
export class UpdaterService implements AppContribution {
  @inject(StatusService)
  protected readonly statusService: StatusService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  async prepare() {
    await this.maybeUnpackTheia();

    // await this.simulateUpdate();
  }

  protected async simulateUpdate(): Promise<void> {
    await new Promise<void>((resolve) => {
      let steps = 0;
      const limit = 100;

      const sendStatus = () => {
        this.statusService.set(updaterStatuses.state, {
          completed: steps / limit,
          currentFileName: `Step ${steps}/${limit}`,
        });
      }

      const int = setInterval(() => {
        if (steps++ >= limit) {
          clearInterval(int);
          return resolve();
        }

        sendStatus();
      }, 50);
    });
  }

  protected async maybeUnpackTheia(): Promise<void> {
    // if not self-hosted - skip
    if (!this.configService.selfHosted) {
      return;
    }

    // No archive - no unpack
    const archiveStats = await this.fsService.statSafe(this.configService.sdkRootTheiaArchive);
    if (!archiveStats) {
      return;
    }

    // archive contains root `build` dir, so when unpacked it will be this path
    const extractedArchivePath = this.fsService.joinPath(this.configService.sdkRoot, 'build');

    const archiveSize = archiveStats.size;
    let unpackedSize = 0;
    let unpackedFiles = 0;
    let currentFileName = '';

    const sendStatus = () => {
      this.statusService.set(updaterStatuses.state, {
        completed: unpackedSize / archiveSize,
        currentFileName,
      });
    }

    // rm theia dir
    if (await this.fsService.statSafe(this.configService.sdkRootTheia)) {
      currentFileName = 'Removing old install...';
      sendStatus();

      await this.fsService.rimraf(this.configService.sdkRootTheia);
    }

    // rm maybe-present build dir, might be interrupted earlier unpack attempt
    if (await this.fsService.statSafe(extractedArchivePath)) {
      await this.fsService.rimraf(extractedArchivePath);
    }

    const unpackDefer = new Deferred();

    const archiveReadStream = this.fsService.createReadStream(this.configService.sdkRootTheiaArchive);
    const extractorStream = tar.extract(this.configService.sdkRoot, {
      mapStream: (fileReadStream, headers) => {
        unpackedFiles++;
        currentFileName = `Unpacking files ${unpackedFiles}`;

        fileReadStream.on('data', (data) => {
          unpackedSize += data.length;

          sendStatus();
        });

        return fileReadStream;
      },
    });

    extractorStream.on('finish', () => unpackDefer.resolve());

    archiveReadStream.pipe(extractorStream);

    // Waiting for unpack to finish
    await unpackDefer.promise;

    currentFileName = 'Finishing...';
    sendStatus();

    // Deleting archive and renaming `build` to `personality-theia`
    await Promise.all([
      this.fsService.unlink(this.configService.sdkRootTheiaArchive),
      this.fsService.rename(extractedArchivePath, this.configService.sdkRootTheia),
    ]);
  }
}
