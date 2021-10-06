import tar from 'tar-fs';
import { inject, injectable } from "inversify";
import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";
import { StatusService } from "backend/status/status-service";
import { UpdaterContribution } from "backend/updater/updater-contribution";
import { updaterStatuses } from "shared/api.statuses";
import { Deferred } from "backend/deferred";
import { UpdaterUtils } from 'backend/updater/updater-utils';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';

@injectable()
export class FXCodeUpdaterContribution implements UpdaterContribution {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(StatusService)
  protected readonly statusService: StatusService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(UpdaterUtils)
  protected readonly updaterUtils: UpdaterUtils;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  async update() {
    await this.maybeUnpack()
      .finally(() => {
        this.statusService.set(updaterStatuses.state, {
          completed: 1,
          currentFileName: 'Ready',
        });
      });

    this.theiaCleanup();
  }

  protected async maybeUnpack() {
    // if not self-hosted - skip
    if (!this.configService.selfHosted) {
      return;
    }

    // No archive - no unpack
    const archiveStats = await this.fsService.statSafe(this.configService.sdkRootFXCodeArchive);
    if (!archiveStats) {
      return;
    }

    // archive contains root `out-fxdk-pkg` dir, so when unpacked it will be this path
    const extractedArchivePath = this.fsService.joinPath(this.configService.sdkRoot, 'out-fxdk-pkg');

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
    if (await this.fsService.statSafe(this.configService.sdkRootFXCode)) {
      currentFileName = 'Removing old install...';
      sendStatus();

      await this.fsService.rimraf(this.configService.sdkRootFXCode);
    }

    // rm maybe-present build dir, might be interrupted earlier unpack attempt
    if (await this.fsService.statSafe(extractedArchivePath)) {
      await this.fsService.rimraf(extractedArchivePath);
    }

    const unpackDefer = new Deferred();

    const archiveReadStream = this.fsService.createReadStream(this.configService.sdkRootFXCodeArchive);
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

    unpackedSize = archiveSize;
    currentFileName = 'Finishing...';
    sendStatus();

    await Promise.all([
      this.fsService.unlink(this.configService.sdkRootFXCodeArchive),
      this.fsService.rename(extractedArchivePath, this.configService.sdkRootFXCode),
    ]);
  }

  protected async theiaCleanup() {
    const theiaStoragePath = this.fsService.joinPath(this.configService.cfxLocalAppData, 'sdk-personality-theia');
    const theiaPluginsPath = this.fsService.joinPath(this.configService.cfxLocalAppData, 'sdk-personality-theia-plugins');

    if (await this.fsService.statSafe(theiaStoragePath)) {
      try {
        await this.fsService.rimraf(theiaStoragePath);
      } catch (e) {
        // don't really care
      }
    }

    if (await this.fsService.statSafe(theiaPluginsPath)) {
      try {
        await this.fsService.rimraf(theiaPluginsPath);
      } catch (e) {
        // don't really care
      }
    }
  }
}
