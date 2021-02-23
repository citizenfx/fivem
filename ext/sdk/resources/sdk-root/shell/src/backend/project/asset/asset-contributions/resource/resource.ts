import { inject, injectable } from "inversify";
import { IMinimatch, Minimatch } from 'minimatch';
import { GameServerService } from "backend/game-server/game-server-service";
import { resourceManifestFilename, resourceManifestLegacyFilename } from 'backend/constants';
import { fastRandomId } from 'utils/random';
import { FsUpdateType } from 'backend/fs/fs-mapping';
import { FilesystemEntry } from "shared/api.types";
import { LogService } from "backend/logger/log-service";
import { FsService } from "backend/fs/fs-service";
import { ProjectAccess } from "backend/project/project-access";
import { ResourceManifest } from "./resource-manifest";
import { ShellCommand } from "backend/process/ShellCommand";
import { NotificationService } from "backend/notification/notification-service";
import { DisposableContainer } from "backend/disposable-container";
import { StatusProxy, StatusService } from "backend/status/status-service";
import { ResourceStatus } from "./resource-types";
import { OutputService } from "backend/output/output-service";
import { uniqueArray } from "utils/unique";
import { Deferred } from "backend/deferred";
import { AssetBuildCommandError } from "../../asset-error";
import { AssetInterface } from "../../asset-types";
import { ProjectManifestResource } from "shared/project.types";


interface IdealResourceMetaData {
  client_script: string[];
  client_scripts: string[];
  shared_script: string[];
  shared_scripts: string[];
  server_script: string[];
  server_scripts: string[];
};

export type ResourceMetaData = Partial<IdealResourceMetaData>;

export const AssetEntry = Symbol('AssetEntry');

const resourceGlobConfig = {
  dot: true,
  nobrace: true,
  nocase: true,
  nocomment: true,
  noext: true,
  nonegate: true,
  nonull: true,
};

@injectable()
export class Resource implements AssetInterface {
  getId() {
    return this.path;
  }

  getName() {
    return this.entry.name;
  }

  getPath() {
    return this.entry.path;
  }

  get name(): string {
    return this.entry.name;
  }

  get path(): string {
    return this.entry.path;
  }

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(StatusService)
  protected readonly statusService: StatusService;

  @inject(OutputService)
  protected readonly outputService: OutputService;

  protected entry: FilesystemEntry;

  protected manifest: ResourceManifest = new ResourceManifest();
  protected manifestPath: string;

  protected metaData: ResourceMetaData = {};
  protected metaDataLoading = true;

  protected restartInducingPaths: string[] = [];
  protected restartInducingPatterns: Record<string, IMinimatch> = {};

  protected buildCommandsDisposableContainer = new DisposableContainer();
  protected disposableContainer = new DisposableContainer();
  protected status: StatusProxy<ResourceStatus>;

  private watchCommandsSuspended = false;
  private runningWatchCommands: Map<string, ShellCommand> = new Map();

  setEntry(assetEntry: FilesystemEntry) {
    this.entry = assetEntry;
  }

  getConfig(): ProjectManifestResource {
    return this.projectAccess.getInstance().getResourceConfig(this.name);
  }

  async getIgnorePatterns(): Promise<string[]> {
    return this.fsService.readIgnorePatterns(this.getPath());
  }

  async getDeployablePaths(): Promise<string[]> {
    const resourcePath = this.getPath();
    const ignorePatterns = await this.getIgnorePatterns();

    const patternPaths = ignorePatterns.concat('**/*')
      .map((pattern) => pattern.replace(/\\/g, '/'));

    const allPaths = await this.fsService.glob(
      patternPaths,
      {
        cwd: resourcePath.replace(/\\/g, '/'),
      },
    );

    return uniqueArray(allPaths.map((pathItem) => this.fsService.joinPath(resourcePath, pathItem)).concat(this.manifestPath));
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

    this.ensureWatchCommandsRunning();
  }

  async runBuildCommands() {
    if (!this.buildCommandsDisposableContainer.empty()) {
      this.buildCommandsDisposableContainer.dispose();
      this.buildCommandsDisposableContainer = new DisposableContainer();
    }

    const commands = this.manifest.fxdkBuildCommands;
    if (!commands.length) {
      return;
    }

    const promises = commands.map(([cmd, args]) => {
      const shellCommand = new ShellCommand(cmd, Array.isArray(args) ? args : [], this.path);
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
              this.name,
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
  }

  async init() {
    this.disposableContainer.add(
      this.status = this.statusService.createProxy(`resource-${this.path}`),
    );

    this.status.setValue({
      watchCommands: {},
    });

    this.logService.log('Resource asset inited', this.name, this.entry);

    this.projectAccess.withInstance((project) => {
      this.disposableContainer.add(project.onResourceConfigChange(this.name, (cfg) => this.onConfigChanged(cfg)));
    });

    const manifestPath = this.fsService.joinPath(this.path, resourceManifestFilename);
    const legacyManifestPath = this.fsService.joinPath(this.path, resourceManifestLegacyFilename);

    if (await this.fsService.statSafe(manifestPath)) {
      this.manifestPath = manifestPath;
    } else if (await this.fsService.statSafe(legacyManifestPath)) {
      this.manifestPath = legacyManifestPath;
    }

    this.loadMetaData();
  }

  async onFsUpdate(updateType: FsUpdateType, entry: FilesystemEntry | null) {
    const isChange = updateType === FsUpdateType.change;

    this.logService.log('fs update', updateType, entry);

    const { enabled, restartOnChange } = this.projectAccess.getInstance().getResourceConfig(this.entry.name);
    if (!enabled) {
      this.logService.log('Resource is disabled', this.name);
      return;
    }

    if (isChange && entry?.path === this.manifestPath) {
      this.logService.log('Reloading meta data as manifest changed', this.name);
      this.loadMetaData();
      return this.gameServerService.refreshResources();
    }

    // No restarts while meta data is being loaded
    if (this.metaDataLoading) {
      this.logService.log('Meta data for resource is still loading, ignoring change fs update', updateType, entry);
      return;
    }

    if (!isChange || !restartOnChange) {
      return;
    }

    const isRestartInducingPath = Object.values(this.restartInducingPatterns)
      .some((pattern) => {
        return pattern.match(entry.path);
      });

    if (isRestartInducingPath) {
      this.logService.log(`[Resource ${this.name}] Restarting resource`, this.name);
      this.gameServerService.restartResource(this.name);
    }
  }

  async dispose() {
    await this.disposableContainer.dispose();

    await Promise.all([...this.runningWatchCommands.values()].map((cmd) => cmd.stop()));

    this.projectAccess.withInstance((project) => project.applyResourcesChange((resources) => {
      delete resources[this.entry.name];
    }));
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

    this.logService.log('Running watch commands of', this.name, this.metaData, this.manifest.fxdkWatchCommands);

    const stubs: Record<string, { command: string, args: unknown }> = this.manifest.fxdkWatchCommands.reduce((acc, [command, args]) => {
      acc[JSON.stringify([command, args])] = { command, args };

      return acc;
    }, {});

    // Killing old promises
    const oldCommandsPromises = [];
    const commandHashesToDelete = [];

    this.runningWatchCommands.forEach((cmd, hash) => {
      if (!stubs[hash]) {
        this.logService.log('Stopping watch command of', this.name, hash);

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

      this.logService.log('Stopping watch command', hash);

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
    const cmd = new ShellCommand(command, Array.isArray(args) ? args : [], this.path);
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
      this.notificationService.error(`Watch command for resource ${this.name} has failed to start: ${err.toString()}`);
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

    this.logService.log(`Started watch command`, this.name, command, args);
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
      ...this.manifest.clientScripts,
      ...this.manifest.serverScripts,
      ...this.manifest.sharedScripts,
    ]);

    this.restartInducingPatterns = {};

    scripts.forEach((script) => {
      const fullScript = this.fsService.joinPath(this.path, script);

      this.restartInducingPatterns[script] = new Minimatch(fullScript, resourceGlobConfig);
    });
  }

  private async onConfigChanged(config: ProjectManifestResource) {
    this.logService.log(`Resource ${this.name} config has changed`, config);

    this.ensureWatchCommandsRunning();
  }

  private async loadMetaData(): Promise<void> {
    this.metaDataLoading = true;

    this.logService.log('Loading resource meta data', this.name);

    return new Promise((resolve, reject) => {
      const uid = this.name + '-' + fastRandomId();
      const timeout = setTimeout(() => {
        this.metaDataLoading = false;
        reject(`Resource ${this.path} meta data load timed out after 5 seconds`);
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

          this.logService.log('Loaded meta data for asset', uid, metaData, this.manifest);

          this.metaData = metaData;
          this.restartInducingPaths = this.manifest.getAllScripts().map(
            (relativePath) => this.fsService.joinPath(this.path, relativePath),
          );

          this.metaDataLoading = false;

          this.rebuildRestartInducingPatterns();
          this.ensureWatchCommandsRunning();

          return resolve();
        }
      };

      on('sdk:resourceMetaDataResponse', cb);
      emit('sdk:requestResourceMetaData', this.path, uid);
    });
  }
}
