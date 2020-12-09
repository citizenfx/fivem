import chokidar from 'chokidar';
import { inject, injectable, named } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from 'backend/api/api-decorators';
import { fxdkAssetFilename, fxdkProjectFilename, resourceManifestFilename, resourceManifestLegacyFilename } from 'backend/constants';
import { EntryMetaExtras, ExplorerService } from 'backend/explorer/explorer-service';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { assetApi, projectApi } from 'shared/api.events';
import {
  AssetCreateRequest,
  AssetDeleteRequest,
  AssetRenameRequest,
  CopyEntryRequest,
  MoveEntryRequest,
  ProjectCreateDirectoryRequest,
  ProjectCreateFileRequest,
  ProjectCreateRequest,
  ProjectDeleteDirectoryRequest,
  ProjectDeleteFileRequest,
  ProjectRenameDirectoryRequest,
  ProjectRenameFileRequest,
  ProjectSetResourceConfigRequest,
} from 'shared/api.requests';
import {
  AssetMeta,
  ProjectData,
  ProjectFsTree,
  ProjectManifest,
  ProjectPathsState,
  ProjectResources,
  ServerUpdateChannel,
  serverUpdateChannels,
} from "shared/api.types";
import { debounce, getEnabledResourcesPaths, getProjectResources, getResourceConfig } from 'shared/utils';
import { ContributionProvider } from 'backend/contribution-provider';
import { AssetKind, AssetManager } from './asset/asset-contribution';
import { GameServerService } from 'backend/game-server/game-server-service';
import { FsAtomicWriter } from 'backend/fs/fs-atomic-writer';
import { SkipRepetitiveExecutor } from 'backend/execution-utils/skip-repetitive-executor';
import { Sequencer } from 'backend/execution-utils/sequencer';

enum FsTreeUpdateType {
  add,
  addDir,
  change,
  unlink,
  unlinkDir,
}

interface Silentable {
  silent?: boolean,
}

export interface ManifestPropagationOptions extends Silentable {
}

export interface AssetMetaAccessorOptions extends Silentable {
  forceShadow?: boolean,
  forceReal?: boolean,
}

export interface GetAssetMetaOptions extends AssetMetaAccessorOptions {
}

export interface SetAssetMetaOptions extends AssetMetaAccessorOptions {
}

@injectable()
export class Project implements ApiContribution {
  getId() {
    return `ProjectService(${this.path})`;
  }

  eventDisposers: Function[] = [];

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ExplorerService)
  protected readonly explorerService: ExplorerService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @inject(ContributionProvider) @named(AssetManager)
  protected readonly assetManagers: ContributionProvider<AssetManager>;

  @inject(ContributionProvider) @named(AssetKind)
  protected readonly assetKinds: ContributionProvider<AssetKind>;

  private path: string;
  private fsTree: ProjectFsTree = undefined as any;
  private manifest: ProjectManifest = undefined as any;
  private resources: ProjectResources = undefined as any;

  private watcher: chokidar.FSWatcher = undefined as any;

  private fsOpsSequencer = new Sequencer();

  get projectData(): ProjectData {
    return {
      path: this.path,
      fsTree: this.fsTree,
      manifest: this.manifest,
    };
  }

  getManifest() {
    return this.manifest;
  }

  getPath(): string {
    return this.path;
  }

  get manifestPath(): string {
    return this.fsService.joinPath(this.path, fxdkProjectFilename);
  }

  get storagePath(): string {
    return this.fsService.joinPath(this.path, '.fxdk');
  }

  get shadowPath(): string {
    return this.fsService.joinPath(this.storagePath, 'shadowRoot');
  }

  get enabledResourcesPaths(): string[] {
    return getEnabledResourcesPaths(this.projectData, this.resources);
  }

  get entryMetaExtras(): EntryMetaExtras {
    return {
      assetMeta: async (entryPath: string) => {
        return this.getAssetMeta(entryPath, { silent: true });
      },
    };
  }

  log(msg: string, ...args) {
    this.logService.log(`[ProjectInstance: ${this.path}]`, msg, ...args);
  }

  async create(request: ProjectCreateRequest): Promise<Project> {
    this.logService.log('Creating project', request);

    this.path = this.fsService.joinPath(request.projectPath, request.projectName);

    const projectShadowRootPath = this.fsService.joinPath(this.path, '.fxdk/shadowRoot');

    const projectManifestPath = this.fsService.joinPath(this.path, fxdkProjectFilename);
    const projectManifest: ProjectManifest = {
      name: request.projectName,
      serverUpdateChannel: serverUpdateChannels.recommended,
      createdAt: new Date().toISOString(),
      resources: {},
      pathsState: {},
    };

    const theiaSettingsPath = this.fsService.joinPath(this.path, '.fxdk/theia-settings.json');
    const theiaSettings = {
      folders: [],
      settings: {},
    };

    // It will create this.path as well
    await this.fsService.mkdirp(projectShadowRootPath)

    await Promise.all([
      // Write project manifest
      this.fsService.writeFile(projectManifestPath, JSON.stringify(projectManifest, null, 2)),

      // Write theia-personality settings
      this.fsService.writeFile(theiaSettingsPath, JSON.stringify(theiaSettings, null, 2)),
    ]);

    this.logService.log('Done creating project', request);

    return this.load();
  }

  async load(path?: string): Promise<Project> {
    if (path) {
      this.path = path;
    }

    this.log('loading project...');

    await Promise.all([
      this.readManifest(),
      this.readFsTree(),
    ]);

    this.watchProject();

    this.setGameServerServiceEnabledResources();

    this.log('done loading project');

    return this;
  }

  async unload() {
    this.log('closing...');

    this.eventDisposers.forEach((disposer) => disposer());

    this.gameServerService.stop();

    if (this.watcher) {
      await this.watcher.close();
    }

    this.log('closed');
  }

  getProjectResources(): ProjectResources {
    return this.resources;
  }

  private async propagateManifestChanges(options?: ManifestPropagationOptions) {
    this.writeManifest();

    if (!options?.silent) {
      this.notifyProjectUpdated();
    }
  }

  async setManifest(manifest: ProjectManifest, options?: ManifestPropagationOptions) {
    this.manifest = manifest;

    this.propagateManifestChanges(options);
  }

  async setResourcesEnabled(resourceNames: string[], enabled: boolean) {
    resourceNames.forEach((resourceName) => {
      this.setResourceConfig({ resourceName, config: { enabled } });
    });
  }

  @handlesClientEvent(projectApi.setResourceConfig)
  async setResourceConfig({ resourceName, config }: ProjectSetResourceConfigRequest) {
    this.manifest.resources[resourceName] = {
      ...getResourceConfig(this.manifest, resourceName),
      ...config,
    };

    this.propagateManifestChanges();
  }

  @handlesClientEvent(projectApi.setPathsState)
  setPathsState(pathsState: ProjectPathsState) {
    this.manifest.pathsState = pathsState;

    this.propagateManifestChanges();
  }

  /**
   * Returns path but in shadow root
   */
  getPathInShadow(requestPath: string): string {
    let relativePath = requestPath;

    if (this.fsService.isAbsolutePath(requestPath)) {
      relativePath = this.fsService.relativePath(this.path, requestPath);
    }

    return this.fsService.joinPath(this.shadowPath, relativePath);
  }

  @handlesClientEvent(projectApi.setServerUpdateChannel)
  setServerUpdateChannel(updateChannel: ServerUpdateChannel) {
    if (this.manifest.serverUpdateChannel !== updateChannel) {
      this.gameServerService.stop();

      this.manifest.serverUpdateChannel = updateChannel;

      this.propagateManifestChanges();
    }
  }

  /**
   * Returns meta filepath that exist in assetPath
   * Otherwise will return meta filepath in shadow root
   *
   * Can be overridden by options
   */
  async getAssetMetaPath(assetPath: string, options?: AssetMetaAccessorOptions): Promise<string> {
    const shadowPath = this.fsService.joinPath(this.getPathInShadow(assetPath), fxdkAssetFilename);
    const realPath = this.fsService.joinPath(assetPath, fxdkAssetFilename);

    if (options?.forceShadow) {
      return shadowPath;
    }

    if (options?.forceReal) {
      return realPath;
    }

    if (await this.fsService.statSafe(realPath)) {
      return realPath;
    }

    return shadowPath;
  }

  async getAssetMeta(assetPath: string, options?: GetAssetMetaOptions): Promise<AssetMeta | null> {
    const assetMetaFilepath = await this.getAssetMetaPath(assetPath, options);

    try {
      await this.fsService.stat(assetMetaFilepath);

      const assetMeta = await this.fsService.readFile(assetMetaFilepath);

      return JSON.parse(assetMeta.toString());
    } catch (e) {
      if (!options?.silent) {
        console.error('Error reading asset meta from file', {
          projectPath: this.path,
          assetMetaFilepath,
        }, e);
      }

      return null;
    }
  }

  async setAssetMeta(assetPath: string, assetMeta: AssetMeta, options?: SetAssetMetaOptions) {
    this.log('Saving asset meta to file', {
      projectPath: this.path,
      assetPath,
      assetMeta,
      options,
    });

    const assetMetaFilepath = await this.getAssetMetaPath(assetPath, options);

    // Ensure directory exist
    await this.fsService.mkdirp(this.fsService.dirname(assetMetaFilepath));

    await this.fsService.writeFile(assetMetaFilepath, JSON.stringify(assetMeta, null, 2));
  }

  // Directories methods
  @handlesClientEvent(projectApi.createDirectory)
  async createDirectory({ directoryName, directoryPath }: ProjectCreateDirectoryRequest) {
    const directoryFullPath = this.fsService.joinPath(directoryPath, directoryName);

    await this.fsService.mkdirp(directoryFullPath);
  }

  @handlesClientEvent(projectApi.deleteDirectory)
  async deleteDirectory({ directoryPath }: ProjectDeleteDirectoryRequest) {
    await this.fsService.rimraf(directoryPath);
  }

  @handlesClientEvent(projectApi.renameDirectory)
  async renameDirectory({ directoryPath, newDirectoryName }: ProjectRenameDirectoryRequest) {
    const newDirectoryPath = this.fsService.joinPath(this.fsService.dirname(directoryPath), newDirectoryName);

    await this.fsService.rename(directoryPath, newDirectoryPath);
  }
  // /Directory methods

  // Files methods
  @handlesClientEvent(projectApi.createFile)
  async createFile({ filePath, fileName }: ProjectCreateFileRequest) {
    const fileFullPath = this.fsService.joinPath(filePath, fileName);

    await this.fsService.writeFile(fileFullPath, '');
  }

  @handlesClientEvent(projectApi.deleteFile)
  async deleteFile({ filePath }: ProjectDeleteFileRequest) {
    await this.fsService.unlink(filePath);
  }

  @handlesClientEvent(projectApi.renameFile)
  async renameFile({ filePath, newFileName }: ProjectRenameFileRequest) {
    const newFilePath = this.fsService.joinPath(this.fsService.dirname(filePath), newFileName);

    await this.fsService.rename(filePath, newFilePath);
  }
  // /Files methods

  // FS methods
  @handlesClientEvent(projectApi.moveEntry)
  async moveEntry(request: MoveEntryRequest) {
    this.fsOpsSequencer.executeParallel(async () => {
      const { sourcePath, targetPath } = request;

      const newPath = this.fsService.joinPath(targetPath, this.fsService.basename(sourcePath));

      if (newPath === sourcePath) {
        return;
      }

      await this.fsService.rename(sourcePath, newPath);
    });
  }

  @handlesClientEvent(projectApi.copyEntry)
  async copyEntry(request: CopyEntryRequest) {
    this.notificationService.warning('Copying is not implemented yet :sob:');
  }
  // /FS methods

  // Asset methods
  protected getAssetKind(kindName: string): AssetKind {
    try {
      return this.assetKinds.getTagged('kindName', kindName);
    } catch (e) {
      throw new Error(`No asset kind of type ${kindName}`);
    }
  }

  protected getAssetManager(managerName: string): AssetManager {
    try {
      return this.assetManagers.getTagged('managerName', managerName);
    } catch (e) {
      throw new Error(`No asset manager of type ${managerName}`);
    }
  }

  @handlesClientEvent(assetApi.create)
  createAsset(request: AssetCreateRequest) {
    this.log('Creating asset', request);

    if (request.managerType) {
      return this.getAssetManager(request.managerType).create(this, request);
    }

    if (request.assetKind) {
      return this.getAssetKind(request.assetKind).create(this, request);
    }

    throw new Error('Invalid asset create request, either assetKind or managerType must be specified');
  }

  @handlesClientEvent(assetApi.rename)
  async renameAsset(request: AssetRenameRequest) {
    this.log('Renaming asset', request);

    // Applying changes to project manifest first
    await this.fsOpsSequencer.executeParallel(async () => {
      const { newAssetName } = request;

      const oldAssetName = this.fsService.basename(request.assetPath);

      const resourceConfig = this.manifest.resources[oldAssetName];
      if (resourceConfig) {
        this.manifest.resources[newAssetName] = {
          ...resourceConfig,
          name: newAssetName,
        };

        await this.writeManifest();
      }
    });

    const { assetPath, newAssetName } = request;

    const newAssetPath = this.fsService.joinPath(this.fsService.dirname(assetPath), newAssetName);

    const promises = [
      this.fsService.rename(assetPath, newAssetPath),
    ];

    const oldShadowAssetPath = this.getPathInShadow(assetPath);
    const newShadowAssetPath = this.getPathInShadow(newAssetPath);

    if (await this.fsService.statSafe(oldShadowAssetPath)) {
      promises.push(
        this.fsService.rename(oldShadowAssetPath, newShadowAssetPath),
      );
    }

    await Promise.all(promises);
  }

  @handlesClientEvent(assetApi.delete)
  async deleteAsset(request: AssetDeleteRequest) {
    this.log('Deleting asset', request);

    // Apply changes to project manifest first
    await this.fsOpsSequencer.executeParallel(async () => {
      const { assetPath } = request;

      const assetName = this.fsService.basename(assetPath);

      const resourceConfig = this.manifest.resources[assetName];
      if (resourceConfig) {
        delete this.manifest.resources[assetName];

        await this.writeManifest();
      }
    });

    const { assetPath } = request;

    await Promise.all([
      this.fsService.rimraf(assetPath),
      this.fsService.rimraf(this.getPathInShadow(assetPath)),
    ]);
  }
  // /Asset methods

  /**
   * Send partial project update
   */
  private notifyProjectUpdated() {
    this.apiClient.emit(projectApi.update, this.projectData);
  }

  private async readManifest(): Promise<ProjectManifest> {
    const manifestContent = await this.fsService.readFile(this.manifestPath);

    return this.manifest = {
      pathsState: {},
      resources: {},
      serverUpdateChannel: serverUpdateChannels.recommended,
      ...JSON.parse(manifestContent.toString('utf8')),
    };
  }

  private _manifestWriter: FsAtomicWriter | void;
  private async writeManifest() {
    if (!this._manifestWriter) {
      this._manifestWriter = this.fsService.createAtomicWrite(this.manifestPath);
    }

    await this._manifestWriter.write(JSON.stringify({
      ...this.manifest,
      updatedAt: new Date().toISOString(),
    }, null, 2));

    this.log('Finished writing manifest!');
  }

  private async watchProject() {
    this.log('Start watching project');

    this.watcher = chokidar.watch(this.path, {
      // Ignoring fxdk system folder of project
      ignored: this.fsService.joinPath(this.path, '.fxdk'),
      persistent: true,
      ignoreInitial: true,
    });

    this.watcher
      .on('add', (updatedPath: string) => this.handleFsTreeUpdate(FsTreeUpdateType.add, updatedPath))
      .on('addDir', (updatedPath: string) => this.handleFsTreeUpdate(FsTreeUpdateType.addDir, updatedPath))
      .on('change', (updatedPath: string) => this.handleFsTreeUpdate(FsTreeUpdateType.change, updatedPath))
      .on('unlink', (updatedPath: string) => this.handleFsTreeUpdate(FsTreeUpdateType.unlink, updatedPath))
      .on('unlinkDir', (updatedPath: string) => this.handleFsTreeUpdate(FsTreeUpdateType.unlinkDir, updatedPath));
  }

  readAndNotifyFsTree = debounce(async () => {
    this.log('reading fs tree, reconciling resources and sending update');

    await this.readFsTree();
    await this.reconcileManifestResources();

    this.notifyProjectUpdated();
  }, 100);

  private async handleFsTreeUpdate(updateType: FsTreeUpdateType, updatedPath: string) {
    this.log('FSTree update', { updateType, updatedPath });

    if (updateType !== FsTreeUpdateType.change) {
      this.readAndNotifyFsTree();
    }

    switch (updateType) {
      case FsTreeUpdateType.change:
      case FsTreeUpdateType.add:
        const updatedPathBaseName = this.fsService.basename(updatedPath);
        if (updatedPathBaseName === resourceManifestFilename || updatedPathBaseName === resourceManifestLegacyFilename) {
          this.log('Resource manifest changed, refreshing');
          this.gameServerService.refreshResources();

          break;
        }

        const enabledResourcesPaths = this.enabledResourcesPaths;
        const updatedResourcePath = enabledResourcesPaths.find((enabledResourcePath) => updatedPath.startsWith(enabledResourcePath));
        if (updatedResourcePath) {
          const resourceName = this.fsService.basename(updatedResourcePath);
          const resourceConfig = getResourceConfig(this.manifest, resourceName);

          if (resourceConfig.restartOnChange) {
            this.gameServerService.restartResource(resourceName);
          }

          break;
        }

        break;
    }
  }

  private async readFsTree(): Promise<ProjectFsTree> {
    const pathsMap = await this.explorerService.readDirRecursively(this.path, this.entryMetaExtras);
    const entries = pathsMap[this.path];

    this.fsTree = {
      entries,
      pathsMap,
    };
    this.resources = getProjectResources(this.projectData);

    return this.fsTree;
  }

  private reconcileManifestResourcesWorker = new SkipRepetitiveExecutor(() => this.fsOpsSequencer.executeBlocking(() => {
    this.log('Reconciling manifest resources');

    const manifestResources = {};

    Object.values(this.resources)
      .forEach(({ name }) => {
        manifestResources[name] = this.manifest.resources[name];
      });

    if (Object.keys(this.manifest.resources).length !== Object.keys(manifestResources).length) {
      this.manifest.resources = manifestResources;
      this.propagateManifestChanges();

      this.setGameServerServiceEnabledResources();
    }
  }));

  private async reconcileManifestResources() {
    this.reconcileManifestResourcesWorker.execute();
  }

  private setGameServerServiceEnabledResources() {
    this.gameServerService.setEnabledResources({
      projectPath: this.path,
      enabledResourcesPaths: this.enabledResourcesPaths,
    });
  }
}
