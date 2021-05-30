import tar from 'tar-fs';
import { inject, injectable } from "inversify";
import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";
import { StatusService } from "backend/status/status-service";
import { UpdaterContribution } from "backend/updater/updater-contribution";
import { updaterStatuses } from "shared/api.statuses";
import { Deferred } from "backend/deferred";
import theiaPlugins from 'backend/theia/theia-plugins';
import { UpdaterUtils } from 'backend/updater/updater-utils';
import { concurrently } from 'utils/concurrently';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';

type Version = string;
type PluginId = string;

type VersionsLock = {
  [pluginId: string]: Version,
};

interface PluginInstallationCandidate {
  pluginId: PluginId,
  version: Version,
  link: string,
}

const versionsLockFileName = 'versions.lock';

@injectable()
export class TheiaUpdaterContribution implements UpdaterContribution {
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
    let pluginsUpdated = false;

    await concurrently(
      this.maybeUpdatePlugins()
        .catch((e) => {
          this.notificationService.error(`Failed to install or update theia plugins: ${e.toString()}`);
        })
        .finally(() => {
          pluginsUpdated = true;
        }),

      this.maybeUnpack()
        .finally(() => {
          if (!pluginsUpdated) {
            this.statusService.set(updaterStatuses.state, {
              completed: 1,
              currentFileName: 'Updating or installing theia plugins',
            });
          }
        }),
    );
  }

  protected async maybeUnpack() {
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

    unpackedSize = archiveSize;
    currentFileName = 'Finishing...';
    sendStatus();

    // Deleting archive and renaming `build` to `personality-theia`
    await Promise.all([
      this.fsService.unlink(this.configService.sdkRootTheiaArchive),
      this.fsService.rename(extractedArchivePath, this.configService.sdkRootTheia),
    ]);
  }

  protected async maybeUpdatePlugins() {
    const versionsLockFilePath = this.fsService.joinPath(this.configService.theiaPluginsPath, versionsLockFileName);

    // Create plugins dir if it does not exist
    await this.fsService.ensureDir(this.configService.theiaPluginsPath);

    // Read versions.lock content
    let versionsLock: VersionsLock | null = null;
    if (await this.fsService.statSafe(versionsLockFilePath)) {
      try {
        const versionsLockFileContent = await this.fsService.readFile(versionsLockFilePath);

        versionsLock = JSON.parse(versionsLockFileContent.toString());
      } catch (e) {
        // Ignore currupted versions.lock file
      }
    }

    // Figure out what plugins we need to install
    const pluginInstallationCandidates: PluginInstallationCandidate[] = Object.entries(theiaPlugins)
      .map(([pluginId, link]) => {
        const version = this.parsePluginVersionFromLink(link);

        return { pluginId, version, link };
      });
    const pluginsToUpdate = pluginInstallationCandidates.filter(({ pluginId, version }) => versionsLock?.[pluginId] !== version);

    // If no plugins require update - finish
    if (pluginsToUpdate.length === 0) {
      return;
    }

    // Install plguins
    await Promise.all(pluginsToUpdate.map(async ({ pluginId, link, version }) => this.installPlugin(pluginId, version, link)));

    // Compose new versions.lock content
    const freshVersionsLock: VersionsLock = pluginInstallationCandidates.reduce((acc, { pluginId, version }) => {
      return Object.assign(acc, { [pluginId]: version });
    }, {});

    // Cleanup
    const pluginDirEntries = await this.fsService.readdir(this.configService.theiaPluginsPath);
    await Promise.all(pluginDirEntries
      .filter((pluginDirEntry) => pluginDirEntry !== versionsLockFileName && !freshVersionsLock[pluginDirEntry])
      .map((pluginDirEntry) => {
        try {
          return this.fsService.rimraf(this.fsService.joinPath(this.configService.theiaPluginsPath, pluginDirEntry));
        } catch(e) {
          // ignore
        }
      },
    ));

    // Save new versions.lock
    await this.fsService.writeFile(versionsLockFilePath, JSON.stringify(freshVersionsLock, null, 2));
  }

  protected async installPlugin(pluginId: PluginId, version: Version, link: string) {
    this.logService.log('Updating theia plugin', { pluginId, link, version });

    const pluginArchivePath = this.fsService.joinPath(this.configService.theiaPluginsPath, pluginId + '.vsix');
    const pluginExtractionPath = this.fsService.joinPath(this.configService.theiaPluginsPath, pluginId);

    await concurrently(
      this.fsService.ensureDeleted(pluginExtractionPath),
      this.fsService.ensureDeleted(pluginArchivePath),
    );

    await this.updaterUtils.downloadFile(pluginArchivePath, link);
    this.logService.log('Downloaded archive', link);

    await this.updaterUtils.unpackZipFile(pluginArchivePath, pluginExtractionPath);
    this.logService.log('Unpacked theia plugin archive', pluginId);

    await this.fsService.unlink(pluginArchivePath);
    this.logService.log('Deleted archive', pluginArchivePath);
  }

  protected parsePluginVersionFromLink(link: string): string {
    const linkWithoutExt = this.fsService.basename(link, '.vsix');

    const idx = linkWithoutExt.lastIndexOf('-');

    return linkWithoutExt.substr(
      idx + 1,
      linkWithoutExt.length - idx - 1,
    );
  }
}
