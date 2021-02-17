import { inject, injectable } from "inversify";
import fetch from 'node-fetch';
import filesize from 'filesize';
import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { AppContribution } from "backend/app/app-contribution";
import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { GameServerInstallerUtils, versionFilename } from "./game-server-installer-utils";
import { ServerUpdateChannel, serverUpdateChannels, ServerUpdateChannelsState, ServerUpdateStates } from "shared/api.types";
import { handlesClientEvent } from "backend/api/api-decorators";
import { serverApi } from "shared/api.events";
import { NotificationService } from "backend/notification/notification-service";
import { TaskReporterService } from "backend/task/task-reporter-service";
import { Deferred } from "backend/deferred";

const updateChannels = Object.keys(serverUpdateChannels);

interface ServerVersions {
  [updateChannel: string]: null | {
    version: string,
    link: string,
  },
}

interface InstalledVersions {
  [updateChannel: string]: string | null,
}

@injectable()
export class GameServerManagerService implements AppContribution, ApiContribution {
  getId() {
    return 'GameServerManagerService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(GameServerInstallerUtils)
  protected readonly gameServerInstallerUtils: GameServerInstallerUtils;

  protected updateChannelsState: ServerUpdateChannelsState = {
    [serverUpdateChannels.recommended]: ServerUpdateStates.checking,
    [serverUpdateChannels.optional]: ServerUpdateStates.checking,
    [serverUpdateChannels.latest]: ServerUpdateStates.checking,
  };

  protected versions: ServerVersions = updateChannels.reduce((acc, updateChannel) => { acc[updateChannel] = null; return acc }, {});
  protected installedVersions: InstalledVersions = updateChannels.reduce((acc, updateChannel) => { acc[updateChannel] = null; return acc }, {});

  protected updateChannelPromises: Set<{ deferred: Deferred<void>, channel: ServerUpdateChannel, state: ServerUpdateStates }> = new Set();

  boot() {
    this.checkAndInstallUpdates();
  }

  getServerPath(updateChannel: ServerUpdateChannel): string {
    return this.fsService.joinPath(this.configService.serverContainer, updateChannel);
  }

  getServerBinaryPath(updateChannel: ServerUpdateChannel): string {
    return this.fsService.joinPath(this.getServerPath(updateChannel), 'FXServer.exe');
  }

  getUpdateChannelPromise(updateChannel: ServerUpdateChannel, targetState: ServerUpdateStates): Promise<void> {
    const currentState = this.updateChannelsState[updateChannel];
    if (currentState === ServerUpdateStates.missingArtifact) {
      throw new Error('Server artifact missing');
    }

    if (currentState === targetState) {
      return Promise.resolve();
    }

    const item = {
      state: targetState,
      channel: updateChannel,
      deferred: new Deferred<void>(),
    };

    this.updateChannelPromises.add(item);

    return item.deferred.promise;
  }

  async getInstalledServerVersion(updateChannel: ServerUpdateChannel): Promise<string | null> {
    const versionFilePath = this.fsService.joinPath(this.getServerPath(updateChannel), versionFilename);
    const versionFileStat = await this.fsService.statSafe(versionFilePath);
    if (!versionFileStat) {
      return null;
    }

    try {
      const versionFileContent = await this.fsService.readFileString(versionFilePath);

      return versionFileContent?.trim() || null;
    } catch (e) {
      return null;
    }
  }

  ensureSvAdhesiveEnabled(updateChannel: ServerUpdateChannel, enabled: boolean) {
    return this.ensureSvAdhesiveEnabledAtPath(this.getServerPath(updateChannel), enabled);
  }

  async ensureSvAdhesiveEnabledAtPath(artifactPath: string, enabled: boolean) {
    const componentsPath = this.fsService.joinPath(artifactPath, 'components.json');
    const componentsContent = await this.fsService.readFile(componentsPath);

    const components: string[] = JSON.parse(componentsContent.toString('utf8'));
    const svadhesiveIndex = components.indexOf('svadhesive');

    // Should be enabled and is enabled -> noop
    if (enabled && svadhesiveIndex > -1) {
      return;
    }
    // Should not be enable and is not enabled -> also noop
    if (!enabled && svadhesiveIndex === -1) {
      return;
    }

    if (enabled) {
      components.push('svadhesive');
    } else {
      components.splice(svadhesiveIndex, 1);
    }

    await this.fsService.writeFile(componentsPath, JSON.stringify(components, null, 2));
  }

  async checkAndInstallUpdates() {
    await Promise.all([
      this.fetchInstalledVersions(),
      this.fetchVersions(),
    ]);

    await Promise.all(
      updateChannels.map((updateChannel) => {
        const installedVersion = this.installedVersions[updateChannel];
        const version = this.versions[updateChannel];

        if (!version) {
          return this.setUpdateChannelState(
            updateChannel,
            installedVersion
              ? ServerUpdateStates.ready
              : ServerUpdateStates.missingArtifact,
          );
        } else if (installedVersion !== version.version) {
          return this.install(updateChannel);
        } else {
          return this.setUpdateChannelState(updateChannel, ServerUpdateStates.ready);
        }
      }),
    );
  }

  async checkForUpdates() {
    const newUpdateChannelsState = { ...this.updateChannelsState };

    this.updateChannelsState = updateChannels.reduce((acc, updateChannel) => {
      acc[updateChannel] = ServerUpdateStates.checking;

      return acc;
    }, {});

    this.ackUpdateChannelsState();

    await this.fetchVersions();

    updateChannels.forEach((updateChannel) => {
      const installedVersion = this.installedVersions[updateChannel];
      const version = this.versions[updateChannel];

      if (version && installedVersion !== version.version) {
        newUpdateChannelsState[updateChannel] = ServerUpdateStates.updateRequired;
      }
    });

    this.updateChannelsState = newUpdateChannelsState;

    this.ackUpdateChannelsState();
  }

  private setUpdateChannelState(updateChannel: ServerUpdateChannel, state: ServerUpdateStates) {
    this.updateChannelsState[updateChannel] = state;

    this.ackUpdateChannelsState();
  }

  /**
   * Fetches installed server versions
   */
  private async fetchInstalledVersions() {
    await Promise.all(
      updateChannels.map(async (updateChannel) => {
        const serverPath = this.getServerPath(updateChannel);
        const serverVersionFilepath = this.fsService.joinPath(serverPath, versionFilename);

        try {
          const versionContent = await this.fsService.readFile(serverVersionFilepath);

          this.installedVersions[updateChannel] = versionContent.toString().trim();
        } catch (e) {
          this.installedVersions[updateChannel] = null;
        }
      }),
    );
  }

  /**
   * Fetches recent server versions from build server
   */
  private async fetchVersions() {
    const fetchTask = this.taskReporterService.create(`Fetching recent server artifact versions`);

    try {
      const versionsContent = await fetch('https://changelogs-live.fivem.net/api/changelog/versions/win32/server').then((res) => res.json());

      updateChannels.forEach((updateChannel) => {
        const versionField = updateChannel;
        const linkField = `${updateChannel}_download`;

        let version;
        let link;

        try {
          version = versionsContent[versionField];
          link = versionsContent[linkField];
        } catch (e) {
          throw new Error(`Malformed response from artifacts server: ${JSON.stringify(versionsContent)}`);
        }

        this.versions[updateChannel] = { version, link };
      });
    } catch (e) {
      updateChannels.forEach((updateChannel) => {
        this.versions[updateChannel] = null;
      });

      return this.logService.log(`Failed to fetch server versions from remote host: ${e.toString()}`);
    } finally {
      fetchTask.done();
    }
  }

  @handlesClientEvent(serverApi.installUpdate)
  private async install(updateChannel: ServerUpdateChannel) {
    const versionConfig = this.versions[updateChannel];
    if (!versionConfig) {
      return;
    }

    this.setUpdateChannelState(updateChannel, ServerUpdateStates.updating);

    await Promise.all([
      this.fsService.mkdirp(this.configService.serverContainer),
      this.fsService.mkdirp(this.configService.serverArtifacts),
      this.deleteOldArtifact(updateChannel),
    ]);

    const { version, link } = versionConfig;

    const artifactPath = this.getArtifactPath(updateChannel, version);
    const artifactExtractionPath = this.getServerPath(updateChannel);
    const downloadingArtifactPath = this.getDownloadingArtifactPath(updateChannel, version);

    if (await this.fsService.statSafe(downloadingArtifactPath)) {
      await this.fsService.unlink(downloadingArtifactPath);
    }

    if (!(await this.fsService.statSafe(artifactPath))) {
      const downloadTask = this.taskReporterService.create(`Downloading server artifact #${updateChannel}`);

      downloadTask.setText(`Downloaded: ${filesize(0)}`);

      let totalDownloadSize = 0;
      let doneDownloadSize = 0;

      try {
        await this.gameServerInstallerUtils.downloadArtifact(
          link,
          downloadingArtifactPath,
          (totalSize) => totalDownloadSize = totalSize,
          (chunkSize) => {
            doneDownloadSize += chunkSize;

            downloadTask.setText(`Downloaded: ${filesize(doneDownloadSize)}`);
            downloadTask.setProgress(doneDownloadSize / totalDownloadSize);
          },
        );
      } catch (e) {
        // Ready as unpacked thing still exist, right?
        this.setUpdateChannelState(updateChannel, ServerUpdateStates.ready);
        this.notificationService.error(`Failed to download server artifact: ${e.toString()}`);
        return;
      } finally {
        downloadTask.done();
      }

      await this.fsService.rename(downloadingArtifactPath, artifactPath);
    }

    if (await this.fsService.statSafe(artifactExtractionPath)) {
      await this.fsService.rimraf(artifactExtractionPath);
    }

    {
      const unpackTask = this.taskReporterService.create(`Unpacking server artifact #${updateChannel}`);

      let totalUnpackSize = 0;
      let doneUnpackSize = 0;

      try {
        await this.gameServerInstallerUtils.unpackArtifact(
          artifactPath,
          artifactExtractionPath,
          (totalSize) => totalUnpackSize = totalSize,
          (chunkSize) => {
            doneUnpackSize += chunkSize;

            unpackTask.setText(`Unpacked: ${filesize(doneUnpackSize)}`);
            unpackTask.setProgress(doneUnpackSize / totalUnpackSize);
          },
        );
      } catch (e) {
        this.setUpdateChannelState(updateChannel, ServerUpdateStates.missingArtifact);
        this.notificationService.error(`Failed to unpack server artifact: ${e.toString()}`);
        return;
      } finally {
        unpackTask.done();
      }
    }

    await this.gameServerInstallerUtils.prepareServer(artifactExtractionPath, version);

    this.installedVersions[updateChannel] = version;

    this.setUpdateChannelState(updateChannel, ServerUpdateStates.ready);
  }

  private async deleteOldArtifact(updateChannel: ServerUpdateChannel) {
    const version = this.installedVersions[updateChannel];
    if (!version) {
      return;
    }

    const artifactPath = this.getArtifactPath(updateChannel, version);

    if (await this.fsService.statSafe(artifactPath)) {
      const deleteTask = this.taskReporterService.create(`Deleting old server artifact #${updateChannel}`);

      try {
        await this.fsService.unlink(artifactPath);
      } catch (e) {
        this.notificationService.error(`Failed to delete old server artifact: ${e.toString()}`);
      } finally {
        deleteTask.done();
      }
    }
  }

  private getArtifactPath(updateChannel: ServerUpdateChannel, version: string) {
    return this.fsService.joinPath(this.configService.serverArtifacts, `${updateChannel}.${version}.zip`);
  }

  private getDownloadingArtifactPath(updateChannel: ServerUpdateChannel, version: string) {
    return this.fsService.joinPath(this.configService.serverArtifacts, `${updateChannel}.${version}.fxdkdownload`);
  }

  @handlesClientEvent(serverApi.ackUpdateChannelsState)
  private ackUpdateChannelsState() {
    this.apiClient.emit(serverApi.updateChannelsState, this.updateChannelsState);

    for (const item of this.updateChannelPromises) {
      const channelState = this.updateChannelsState[item.channel];
      if (channelState === ServerUpdateStates.missingArtifact) {
        this.updateChannelPromises.delete(item);
        item.deferred.reject(new Error('Server artifact missing'));
      } else if(item.state === channelState) {
        this.updateChannelPromises.delete(item);
        item.deferred.resolve();
      }
    }
  }
}
