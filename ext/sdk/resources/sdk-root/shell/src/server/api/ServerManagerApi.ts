import * as fs from 'fs';
import * as path from 'path';
import * as mkdirp from 'mkdirp';
import * as rimrafSync from 'rimraf';
import fetch from 'node-fetch';
import { promisify } from 'util';
import { ApiClient, ServerInstallationState, ServerUpdateChannel, serverUpdateChannels, ServerUpdateChannelsState, ServerUpdateStates } from 'shared/api.types';
import * as paths from '../paths';
import { downloadArtifact, prepareServer, unpackArtifact, versionFilename } from '../serverInstaller';
import { serverApi } from 'shared/api.events';
import { NotificationsApi } from './NotificationsApi';

const rimraf = promisify(rimrafSync);

const updateChannels = Object.keys(serverUpdateChannels);


async function doesPathExist(entrypath: string): Promise<boolean> {
  try {
    await fs.promises.stat(entrypath);

    return true;
  } catch (e) {
    return false;
  }
}


interface ServerVersions {
  [updateChannel: string]: null | {
    version: string,
    link: string,
  },
}

interface InstalledVersions {
  [updateChannel: string]: string | null,
}

export class ServerManagerApi {
  public updateChannelsState: ServerUpdateChannelsState = {
    [serverUpdateChannels.recommended]: ServerUpdateStates.checking,
    [serverUpdateChannels.optional]: ServerUpdateStates.checking,
    [serverUpdateChannels.latest]: ServerUpdateStates.checking,
  };

  private installationState: ServerInstallationState = {
    [serverUpdateChannels.recommended]: null,
    [serverUpdateChannels.optional]: null,
    [serverUpdateChannels.latest]: null,
  };

  private versions: ServerVersions = updateChannels.reduce((acc, updateChannel) => { acc[updateChannel] = null; return acc }, {});
  private installedVersions: InstalledVersions = updateChannels.reduce((acc, updateChannel) => { acc[updateChannel] = null; return acc }, {});

  constructor(
    private readonly client: ApiClient,
    private readonly notifications: NotificationsApi,
  ) {
    this.client.on(serverApi.installUpdate, (updateChannel: ServerUpdateChannel) => this.install(updateChannel));
    this.client.on(serverApi.ackInstallationState, () => this.ackInstallationState());
    this.client.on(serverApi.ackUpdateChannelsState, () => this.ackUpdateChannelsState());

    this.checkAndInstallUpdates();
  }

  getServerPath(updateChannel: ServerUpdateChannel): string {
    return path.join(paths.serverContainer, updateChannel);
  }

  getServerBinaryPath(updateChannel: ServerUpdateChannel): string {
    return path.join(this.getServerPath(updateChannel), 'FXServer.exe');
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
        const serverVersionFilepath = path.join(serverPath, versionFilename);

        try {
          const versionContent = await fs.promises.readFile(serverVersionFilepath);

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
    try {
      const versionsContent = await fetch('https://changelogs-live.fivem.net/api/changelog/versions/win32/server').then((res) => res.json());

      updateChannels.forEach((updateChannel) => {
        const versionField = updateChannel;
        const linkField = `${updateChannel}_download`;

        this.versions[updateChannel] = {
          version: versionsContent[versionField],
          link: versionsContent[linkField],
        };
      });
    } catch (e) {
      updateChannels.forEach((updateChannel) => {
        this.versions[updateChannel] = null;
      });

      this.notifications.error(`Failed to fetch server versions from remote host: ${e.toString()}`);
    }
  }

  private async install(updateChannel: ServerUpdateChannel) {
    const versionConfig = this.versions[updateChannel];

    if (!versionConfig) {
      return;
    }

    this.setUpdateChannelState(updateChannel, ServerUpdateStates.updating);

    await Promise.all([
      mkdirp(paths.serverArtifacts),
      mkdirp(paths.serverContainer),
      this.deleteOldArtifact(updateChannel),
    ]);

    const installationState = {
      downloadedPercentage: 0,
      unpackedPercentage: 0,
    };

    this.installationState[updateChannel] = installationState;
    this.ackInstallationState();

    const { version, link } = versionConfig;

    const artifactPath = this.getArtifactPath(updateChannel, version);
    const artifactExtractionPath = this.getServerPath(updateChannel);
    const downloadingArtifactPath = this.getDownloadingArtifactPath(updateChannel, version);

    if (await doesPathExist(downloadingArtifactPath)) {
      await fs.promises.unlink(downloadingArtifactPath);
    }

    if (!(await doesPathExist(artifactPath))) {
      let totalDownloadSize = 0;
      let doneDownloadSize = 0;

      try {
        await downloadArtifact(
          link,
          downloadingArtifactPath,
          (totalSize) => totalDownloadSize = totalSize,
          (chunkSize) => {
            doneDownloadSize += chunkSize;

            installationState.downloadedPercentage = doneDownloadSize / totalDownloadSize;
            this.ackInstallationState();
          },
        );
      } catch (e) {
        // Ready as unpacked thing still exist
        this.setUpdateChannelState(updateChannel, ServerUpdateStates.ready);
        this.notifications.error(`Failed to download server artifact: ${e.toString()}`);
        return;
      }

      await fs.promises.rename(downloadingArtifactPath, artifactPath);
    } else {
      installationState.downloadedPercentage = 1;
      this.ackInstallationState();
    }

    if (await doesPathExist(artifactExtractionPath)) {
      await rimraf(artifactExtractionPath);
    }

    {
      let totalUnpackSize = 0;
      let doneUnpackSize = 0;

      try {
        await unpackArtifact(
          artifactPath,
          artifactExtractionPath,
          (totalSize) => totalUnpackSize = totalSize,
          (chunkSize) => {
            doneUnpackSize += chunkSize;

            installationState.unpackedPercentage = doneUnpackSize / totalUnpackSize;
            this.ackInstallationState();
          },
        );
      } catch (e) {
        this.setUpdateChannelState(updateChannel, ServerUpdateStates.missingArtifact);
        this.notifications.error(`Failed to unpack server artifact: ${e.toString()}`);
        return;
      }
    }

    await prepareServer(artifactExtractionPath, version);

    this.installedVersions[updateChannel] = version;

    this.setUpdateChannelState(updateChannel, ServerUpdateStates.ready);
  }

  private async deleteOldArtifact(updateChannel: ServerUpdateChannel) {
    const version = this.installedVersions[updateChannel];
    if (!version) {
      return;
    }

    const artifactPath = this.getArtifactPath(updateChannel, version);

    if (await doesPathExist(artifactPath)) {
      await fs.promises.unlink(artifactPath);
    }
  }

  private getArtifactPath(updateChannel: ServerUpdateChannel, version: string) {
    return path.join(paths.serverArtifacts, `${updateChannel}.${version}.zip`);
  }

  private getDownloadingArtifactPath(updateChannel: ServerUpdateChannel, version: string) {
    return path.join(paths.serverArtifacts, `${updateChannel}.${version}.fxdkdownload`);
  }

  private ackInstallationState() {
    this.client.emit(serverApi.installationState, this.installationState);
  }

  private ackUpdateChannelsState() {
    this.client.emit(serverApi.updateChannelsState, this.updateChannelsState);
  }
}
