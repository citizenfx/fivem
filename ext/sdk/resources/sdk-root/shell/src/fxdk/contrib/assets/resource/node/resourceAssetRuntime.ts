import { IMinimatch, Minimatch } from 'minimatch';
import { resourceManifestFilename, resourceManifestLegacyFilename } from 'backend/constants';
import { fastRandomId } from 'utils/random';
import { ResourceManifest } from "../common/resourceManifest";
import { ShellCommand } from "backend/process/ShellCommand";
import { Disposer } from "fxdk/base/disposable";
import { StatusProxy } from "backend/status/status-service";
import { uniqueArray } from "utils/unique";
import { Deferred } from "backend/deferred";
import { ResourceAssetConfig, ResourceStatus } from "../common/resource.types";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { ProjectFsEvents } from "fxdk/project/node/project-events";
import { AssetBuildCommandError, AssetDeployablePathsDescriptor, AssetRuntime, AssetRuntimeDataSource } from "fxdk/project/node/asset/asset-runtime";
import { IResourceRuntimeData } from '../common/resourceRuntimeData';


interface IdealResourceMetaData {
  client_script: string[];
  client_scripts: string[];
  shared_script: string[];
  shared_scripts: string[];
  server_script: string[];
  server_scripts: string[];
};

export type ResourceMetaData = Partial<IdealResourceMetaData>;

const resourceGlobConfig = {
  dot: true,
  nobrace: true,
  nocase: true,
  nocomment: true,
  noext: true,
  nonegate: true,
  nonull: true,
};

export class ResourceAssetRuntime extends AssetRuntime<ResourceAssetConfig> {
  protected manifest: ResourceManifest = new ResourceManifest();
  protected manifestPath: string;

  protected metaData: ResourceMetaData = {};
  protected metaDataLoading = true;

  protected restartInducingPaths: string[] = [];
  protected restartInducingPatterns: Record<string, IMinimatch> = {};

  protected buildCommandsDisposableContainer = new Disposer();
  protected status: StatusProxy<ResourceStatus>;

  protected toDispose = new Disposer();

  private watchCommandsSuspended = false;
  private runningWatchCommands: Map<string, ShellCommand> = new Map();

  private runtimeData = new AssetRuntimeDataSource<IResourceRuntimeData>(this.fsEntryPath, {
    ready: false,
  });

  protected override postContstruct() {
    this.resourceDescriptor = {
      name: this.getName(),
      path: this.fsEntryPath,
    };
  }

  protected override getDefaultConfig() {
    return {
      enabled: false,
      restartOnChange: false,
    };
  }

  private resourceDescriptor: ServerResourceDescriptor;
  getResourceDescriptor() {
    return this.resourceDescriptor;
  }

  async getIgnorePatterns(): Promise<string[]> {
    return this.fsService.readIgnorePatterns(this.fsEntryPath);
  }

  async getDeployablePathsDescriptor(): Promise<AssetDeployablePathsDescriptor> {
    const resourcePath = this.fsEntryPath;
    const ignorePatterns = await this.getIgnorePatterns();

    // Mark all patterns from `.fxdkignore` as exclusions and add others
    const patternPaths = ignorePatterns
      .map((pattern) => '!' + pattern.replace(/\\/g, '/'))
      .concat('**/*');

    const allPaths = await this.fsService.glob(
      patternPaths,
      {
        cwd: resourcePath.replace(/\\/g, '/'),
      },
    );

    return {
      root: resourcePath,
      paths: uniqueArray([
        ...allPaths,
        this.fsService.relativePath(resourcePath, this.manifestPath),
      ]),
    };
  }

  async suspendWatchCommands() {
    if (this.watchCommandsSuspended) {
      return;
    }

    this.watchCommandsSuspended = true;

    await this.stopAllWatchCommands();
  }

  resumeWatchCommands() {
    if (!this.watchCommandsSuspended) {
      return;
    }

    this.watchCommandsSuspended = false;

    return this.ensureWatchCommandsRunning();
  }

  async build() {
    if (!this.buildCommandsDisposableContainer.empty()) {
      this.buildCommandsDisposableContainer.dispose();
      this.buildCommandsDisposableContainer = new Disposer();
    }

    const commands = this.manifest.fxdkBuildCommands;
    if (!commands.length) {
      return;
    }

    await this.suspendWatchCommands();

    const promises = commands.map(([cmd, args]) => {
      const shellCommand = new ShellCommand(cmd, Array.isArray(args) ? args : [], this.fsEntryPath);
      const shellCommandOutputChannel = this.outputService.createOutputChannelFromProvider(shellCommand);

      this.buildCommandsDisposableContainer.add(shellCommandOutputChannel);

      const outputId = shellCommand.getOutputChannelId();
      const deferred = new Deferred();

      let closed = false;

      shellCommand.onClose((code) => {
        if (!closed) {
          closed = true;

          if (code === 0) {
            deferred.resolve();
          } else {
            deferred.reject(new AssetBuildCommandError(
              this.getName(),
              outputId,
            ));
          }
        }
      });

      shellCommand.onError((error) => {
        if (!closed) {
          closed = true;
          deferred.reject(error);
        }
      });

      shellCommand.start();

      return deferred.promise;
    });

    await Promise.all(promises);

    await this.resumeWatchCommands();
  }

  async init() {
    this.status = this.toDispose.register(
      this.statusService.createProxy(`resource-${this.fsEntryPath}`),
    );

    this.status.setValue({
      watchCommands: {},
    });

    const manifestPath = this.fsService.joinPath(this.fsEntryPath, resourceManifestFilename);
    const legacyManifestPath = this.fsService.joinPath(this.fsEntryPath, resourceManifestLegacyFilename);

    if (await this.fsService.statSafe(manifestPath)) {
      this.manifestPath = manifestPath;
    } else if (await this.fsService.statSafe(legacyManifestPath)) {
      this.manifestPath = legacyManifestPath;
    }

    this.loadMetaData();

    this.runtimeData.apply((data) => {
      data.ready = true;
    });
  }

  handleConfigChanged(config: ResourceAssetConfig) {
    this.ensureWatchCommandsRunning();
  }

  async acceptNestedFsEntryUpdated(event: ProjectFsEvents.FsEntryUpdatedEvent) {
    if (this.manifestPath === event.entryPath) {
      this.loadMetaData();

      return this.gameServerService.reloadResource(this.getName());
    }

    if (this.metaDataLoading) {
      return;
    }

    const { enabled, restartOnChange } = this.getConfig();
    if (enabled && restartOnChange) {
      const isRestartInducingPath = Object.values(this.restartInducingPatterns)
        .some((pattern) => pattern.match(event.entryPath));

      if (isRestartInducingPath) {
        this.gameServerService.restartResource(this.getName());
      }
    }
  }

  async dispose() {
    await Promise.all([...this.runningWatchCommands.values()].map((cmd) => cmd.stop()));

    await this.toDispose.dispose();
  }

  private async ensureWatchCommandsRunning() {
    if (this.watchCommandsSuspended) {
      return;
    }

    const { enabled, restartOnChange } = this.getConfig();
    if (!enabled || !restartOnChange) {
      this.stopAllWatchCommands();

      return;
    }

    const stubs: Record<string, { command: string, args: unknown }> = this.manifest.fxdkWatchCommands.reduce((acc, [command, args]) => {
      acc[JSON.stringify([command, args])] = { command, args };

      return acc;
    }, {});

    // Killing old promises
    const oldCommandsPromises: Promise<any>[] = [];
    const commandHashesToDelete: string[] = [];

    this.runningWatchCommands.forEach((cmd, hash) => {
      if (!stubs[hash]) {
        oldCommandsPromises.push(cmd.stop());
        commandHashesToDelete.push(hash);

        this.runningWatchCommands.delete(hash);
      }
    });

    this.status.applyValue((status) => {
      if (status) {
        commandHashesToDelete.forEach((hash) => status.watchCommands[hash]);
      }

      return status;
    });

    await Promise.all(oldCommandsPromises);

    // Starting commands
    Object.entries(stubs).map(([hash, { command, args }]) => {
      if (!this.runningWatchCommands.has(hash)) {
        return this.startWatchCommand(hash, command, args);
      }
    });
  }

  private async stopWatchCommand(hash: string) {
    const cmd = this.runningWatchCommands.get(hash);
    if (cmd) {
      this.runningWatchCommands.delete(hash);

      await cmd.stop();

      this.status.applyValue((status) => {
        if (status) {
          status.watchCommands[hash].running = false;
        }

        return status;
      });
    }
  }

  private async startWatchCommand(hash: string, command: string, args: unknown) {
    const cmd = new ShellCommand(command, Array.isArray(args) ? args : [], this.fsEntryPath);
    const cmdChannel = this.outputService.createOutputChannelFromProvider(cmd);
    const outputId = cmd.getOutputChannelId();

    this.runningWatchCommands.set(hash, cmd);

    cmd.onClose(() => {
      cmdChannel.dispose();

      this.status.applyValue((status) => {
        if (status) {
          status.watchCommands[hash] = {
            outputChannelId: outputId,
            running: false,
          };
        }

        return status;
      });
    });

    cmd.onError((err) => {
      this.notificationService.error(`Watch command for resource ${this.getName()} has failed to start: ${err.toString()}`);
    });

    cmd.start();

    this.status.applyValue((status) => {
      if (status) {
        status.watchCommands[hash] = {
          outputChannelId: outputId,
          running: true,
        };
      }

      return status;
    });
  }

  private async stopAllWatchCommands() {
    if (this.runningWatchCommands.size === 0) {
      return;
    }

    const promises: Promise<void>[] = [];

    this.runningWatchCommands.forEach((_cmd, hash) => {
      promises.push(this.stopWatchCommand(hash));
    });

    await Promise.all(promises);
  }

  private rebuildRestartInducingPatterns() {
    const scripts = new Set([
      ...this.manifest.getAllScripts(),
      ...this.manifest.getFiles(),
    ]);

    this.restartInducingPatterns = {};

    scripts.forEach((script) => {
      const fullScript = this.fsService.joinPath(this.fsEntryPath, script);

      this.restartInducingPatterns[script] = new Minimatch(fullScript, resourceGlobConfig);
    });
  }

  private async loadMetaData(): Promise<void> {
    this.metaDataLoading = true;

    return new Promise((resolve, reject) => {
      const uid = this.getName() + '-' + fastRandomId();
      const timeout = setTimeout(() => {
        this.metaDataLoading = false;
        reject(`Resource ${this.fsEntryPath} meta data load timed out after 5 seconds`);
      }, 5000);

      const cb = (resName: string, metaData: ResourceMetaData) => {
        if (resName === uid) {
          clearTimeout(timeout);
          RemoveEventHandler('sdk:resourceMetaDataResponse', cb);

          try {
            this.manifest.fromObject(metaData);
          } catch (e) {
            this.logService.log('Failed to populate manifest', e.toString());
            this.metaDataLoading = false;
            return;
          }

          this.metaData = metaData;
          this.metaDataLoading = false;

          this.runtimeData.apply((data) => {
            data.convarCategories = this.manifest.convarCategories;
          });

          this.rebuildRestartInducingPatterns();
          this.ensureWatchCommandsRunning();

          return resolve();
        }
      };

      on('sdk:resourceMetaDataResponse', cb);
      emit('sdk:requestResourceMetaData', this.fsEntryPath, uid);
    });
  }
}
