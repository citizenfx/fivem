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
  [updateChannel: string]: {
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

  private versions: ServerVersions | void = undefined;
  private installedVersions: InstalledVersions | void = undefined;

  constructor(
    private readonly client: ApiClient,
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
        if (this.installedVersions[updateChannel] !== this.versions[updateChannel].version) {
          return this.install(updateChannel);
        } else {
          this.updateChannelsState[updateChannel] = ServerUpdateStates.ready;
          this.ackUpdateChannelsState();
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
      if (this.installedVersions[updateChannel] !== this.versions[updateChannel].version) {
        newUpdateChannelsState[updateChannel] = ServerUpdateStates.updateRequired;
      }
    });

    this.updateChannelsState = newUpdateChannelsState;

    this.ackUpdateChannelsState();
  }

  /**
   * Fetches installed server versions
   */
  private async fetchInstalledVersions() {
    this.installedVersions = {};

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
    const versionsContent = await fetch('https://changelogs-live.fivem.net/api/changelog/versions/win32/server').then((res) => res.json());

    this.versions = {};

    updateChannels.forEach((updateChannel) => {
      const versionField = updateChannel;
      const linkField = `${updateChannel}_download`;

      this.versions[updateChannel] = {
        version: versionsContent[versionField],
        link: versionsContent[linkField],
      };
    });
  }

  private async install(updateChannel: ServerUpdateChannel) {
    this.updateChannelsState[updateChannel] = ServerUpdateStates.updating;
    this.ackUpdateChannelsState();

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

    const { version, link } = this.versions[updateChannel];

    const artifactPath = this.getArtifactPath(updateChannel, version);
    const artifactExtractionPath = this.getServerPath(updateChannel);
    const downloadingArtifactPath = this.getDownloadingArtifactPath(updateChannel, version);

    if (await doesPathExist(downloadingArtifactPath)) {
      await fs.promises.unlink(downloadingArtifactPath);
    }

    if (!(await doesPathExist(artifactPath))) {
      let totalDownloadSize = 0;
      let doneDownloadSize = 0;

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
    }

    await prepareServer(artifactExtractionPath, version);

    this.installedVersions[updateChannel] = version;

    this.updateChannelsState[updateChannel] = ServerUpdateStates.ready;
    this.ackUpdateChannelsState();
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
