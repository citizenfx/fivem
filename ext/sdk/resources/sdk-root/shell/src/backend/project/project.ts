import { inject, injectable, named } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientCallbackEvent, handlesClientEvent } from 'backend/api/api-decorators';
import { fxdkProjectFilename } from 'backend/constants';
import { ExplorerService } from 'backend/explorer/explorer-service';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { assetApi, projectApi } from 'shared/api.events';
import {
  AssetCreateRequest,
  AssetDeleteRequest,
  AssetDeleteResponse,
  AssetImportRequest,
  AssetRenameRequest,
  CopyEntriesRequest,
  CopyEntryRequest,
  MoveEntryRequest,
  ProjectCreateDirectoryRequest,
  ProjectCreateFileRequest,
  ProjectCreateRequest,
  DeleteDirectoryRequest,
  DeleteFileRequest,
  ProjectRenameDirectoryRequest,
  ProjectRenameFileRequest,
  ProjectSetAssetConfigRequest,
  DeleteDirectoryResponse,
  DeleteFileResponse,
  ProjectStartServerRequest,
} from 'shared/api.requests';
import {
  FilesystemEntry,
  FilesystemEntryMap,
  ServerUpdateChannel,
  serverUpdateChannels,
} from "shared/api.types";
import { debounce } from 'shared/utils';
import { ContributionProvider } from 'backend/contribution-provider';
import { AssetManagerContribution } from './asset/asset-manager-contribution';
import { GameServerService } from 'backend/game-server/game-server-service';
import { FsJsonFileMapping, FsJsonFileMappingOptions } from 'backend/fs/fs-json-file-mapping';
import { FsMapping } from 'backend/fs/fs-mapping';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { TheiaService } from 'backend/theia/theia-service';
import { getAssetsPriorityQueue } from './project-utils';
import { AssetImporterType, AssetMeta, assetMetaFileExt } from 'shared/asset.types';
import { AssetImporterContribution } from './asset/asset-importer-contribution';
import { AssetInterface } from '../../assets/core/asset-interface';
import { ProjectAssetBaseConfig, ProjectData, ProjectManifest, ProjectPathsState } from 'shared/project.types';
import { isAssetMetaFile, stripAssetMetaExt } from 'utils/project';
import { ProjectUpgrade } from './project-upgrade';
import { ServerResourceDescriptor, ServerStartRequest } from 'backend/game-server/game-server-runtime';
import { ProjectAssets } from './project-assets';
import { ProjectAssetManagers } from './project-asset-managers';
import { disposableFromFunction, DisposableObject } from 'backend/disposable-container';
import { WorldEditorService } from 'backend/world-editor/world-editor-service';
import { SystemResource } from 'backend/system-resources/system-resources-constants';
import { SystemResourcesService } from 'backend/system-resources/system-resources-service';
import { DEFAULT_PROJECT_SYSTEM_RESOURCES } from './project-constants';

interface Silentable {
  silent?: boolean,
}

export type AssetConfigChangedListener = <T extends ProjectAssetBaseConfig>(config: T) => void;

export interface ManifestPropagationOptions extends Silentable {
}

export interface GetAssetMetaOptions extends Silentable {
}

export interface SetAssetMetaOptions extends Silentable {
}

export enum ProjectState {
  Development,
  Building,
}

@injectable()
export class Project implements ApiContribution {
  getId() {
    return `ProjectService(${this.path || '#loading#'})`;
  }

  eventDisposers: Function[] = [];

  protected ready: boolean = false;

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

  @inject(ContributionProvider) @named(AssetManagerContribution)
  protected readonly assetManagerContributions: ContributionProvider<AssetManagerContribution>;

  @inject(ContributionProvider) @named(AssetImporterContribution)
  protected readonly assetImporterContributions: ContributionProvider<AssetImporterContribution>;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(TheiaService)
  protected readonly theiaService: TheiaService;

  @inject(ProjectUpgrade)
  protected readonly projectUpgrade: ProjectUpgrade;

  @inject(ProjectAssets)
  protected readonly assets: ProjectAssets;

  @inject(ProjectAssetManagers)
  protected readonly assetManagers: ProjectAssetManagers;

  @inject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @inject(FsMapping)
  public readonly fsMapping: FsMapping;

  @inject(SystemResourcesService)
  protected readonly systemResourcesService: SystemResourcesService;

  private state: ProjectState = ProjectState.Development;

  private path: string;
  private manifestMapping: FsJsonFileMapping<ProjectManifest>;
  private manifestPath: string;
  private storagePath: string;
  private fxserverCwd: string;

  private assetConfigChangeListeners: Record<string, AssetConfigChangedListener> = Object.create(null);

  onAssetConfigChanged(assetPath: string, listener: AssetConfigChangedListener): DisposableObject {
    this.assetConfigChangeListeners[assetPath] = listener;

    return disposableFromFunction(() => delete this.assetConfigChangeListeners[assetPath]);
  }

  applyManifest(fn: (manifest: ProjectManifest) => void) {
    this.manifestMapping.apply(fn);
  }

  getManifest(): ProjectManifest {
    return this.manifestMapping.get();
  }

  getName(): string {
    return this.getManifest().name;
  }

  getPath(): string {
    return this.path;
  }

  getFs(): FilesystemEntryMap {
    return this.fsMapping.getMap();
  }

  getAssets(): ProjectAssets {
    return this.assets;
  }

  getProjectData(): ProjectData {
    const assets = {};
    const assetTypes = {};

    for (const assetPath of this.getAssets().getAllPaths()) {
      assets[assetPath] = this.getAssetConfig(assetPath);
      assetTypes[assetPath] = this.assets.get(assetPath).type;
    }

    return {
      assets,
      assetTypes,
      fs: this.getFs(),
      path: this.getPath(),
      manifest: this.getManifest(),
    };
  }

  getEnabledAssets(): AssetInterface[] {
    const enabledAssets: AssetInterface[] = [];

    for (const [assetRelativePath, assetConfig] of Object.entries(this.getManifest().assets)) {
      const assetPath = this.fsService.joinPath(this.path, assetRelativePath);
      const asset = this.assets.get(assetPath);

      if (asset && assetConfig.enabled) {
        enabledAssets.push(asset);
      }
    }

    return enabledAssets;
  }

  getFxserverCwd(): string {
    return this.fxserverCwd;
  }

  getUpdateChannel(): ServerUpdateChannel {
    return this.manifestMapping.get().serverUpdateChannel;
  }

  hasAsset(assetPath: string): boolean {
    return !!this.assets.get(assetPath);
  }

  isAsset<T extends new () => AssetInterface>(assetPath: string, ctor: T): boolean {
    const asset = this.assets.get(assetPath);

    return asset && (asset instanceof ctor);
  }

  log(msg: string, ...args) {
    this.logService.log(`[ProjectInstance: ${this.path}]`, msg, ...args);
  }

  enterState(state: ProjectState) {
    this.state = state;
  }

  isNotInDevState(): boolean {
    return this.state !== ProjectState.Development;
  }

  async runBuildCommands() {
    const priorityQueue = getAssetsPriorityQueue(this.assets.getAllPaths());

    for (const assetPathsBatch of priorityQueue) {
      await Promise.all(assetPathsBatch.map((assetPath) => {
        const asset = this.assets.get(assetPath);
        if (!asset?.build) {
          return;
        }

        return asset.build();
      }));
    }
  }

  //#region lifecycle

  async create(request: ProjectCreateRequest): Promise<Project> {
    this.logService.log('Creating project', request);

    const creatingTask = this.taskReporterService.createNamed(projectCreatingTaskName, `Creating project ${request.projectName}`);

    try {
      this.path = this.fsService.resolvePath(this.fsService.joinPath(request.projectPath, request.projectName));
      this.storagePath = this.fsService.joinPath(this.path, '.fxdk');

      const projectManifestPath = this.fsService.joinPath(this.path, fxdkProjectFilename);
      const projectManifest: ProjectManifest = {
        name: request.projectName,
        assets: {},
        createdAt: new Date().toISOString(),
        updatedAt: new Date().toISOString(),
        pathsState: {},
        serverUpdateChannel: serverUpdateChannels.recommended,
        systemResources: DEFAULT_PROJECT_SYSTEM_RESOURCES,
      };

      // This will create this.path as well
      await this.fsService.mkdirp(this.storagePath);

      await Promise.all([
        // Write project manifest
        this.fsService.writeFile(projectManifestPath, JSON.stringify(projectManifest, null, 2)),

        // Write theia-personality settings
        this.theiaService.createDefaultProjectSettings(this.storagePath),
      ]);

      this.logService.log('Done creating project', request);
    } catch (e) {
      throw e;
    } finally {
      creatingTask.done();
    }

    return this.load(false);
  }

  open(projectPath: string): Promise<Project> {
    this.path = this.fsService.resolvePath(projectPath);

    return this.load(true);
  }

  private async load(runUpgradeRoutines: boolean): Promise<Project> {
    const loadTask = this.taskReporterService.createNamed(projectLoadingTaskName, `Loading project ${this.path}`);

    this.manifestPath = this.fsService.joinPath(this.path, fxdkProjectFilename);
    this.storagePath = this.fsService.joinPath(this.path, '.fxdk');
    this.fxserverCwd = this.fsService.joinPath(this.storagePath, 'fxserver');

    try {
      this.log('loading project...');

      if (runUpgradeRoutines) {
        await this.projectUpgrade.maybeUpgradeProject({
          task: loadTask,
          projectPath: this.path,
          manifestPath: this.manifestPath,
          storagePath: this.storagePath,
        });
      }

      loadTask.setText('Ensuring fxserver cwd exists...');
      if (!await this.fsService.statSafe(this.fxserverCwd)) {
        await this.fsService.mkdirp(this.fxserverCwd);
      }

      loadTask.setText('Loading manifest...');
      await this.initManifest();

      loadTask.setText('Loading project files...');
      await this.initFs();

      this.refreshEnabledResources();

      loadTask.setText('Done loading project');

      this.log('done loading project');
      this.ready = true;
    } catch (e) {
      throw e;
    } finally {
      loadTask.done();
    }

    return this;
  }

  private async initManifest() {
    const options: FsJsonFileMappingOptions<ProjectManifest> = {
      defaults: {
        assets: {},
        pathsState: {},
        serverUpdateChannel: serverUpdateChannels.recommended,
        systemResources: [],
      },
      onApply: () => this.notifyProjectUpdated(),
      beforeApply: (snapshot) => snapshot.updatedAt = new Date().toISOString(),
    };

    this.manifestMapping = await this.fsService.createJsonFileMapping<ProjectManifest>(this.manifestPath, options);
  }

  private async initFs() {
    this.log('Initializing project fs...');

    this.fsMapping.setShouldProcessUpdate((path) => path !== this.manifestPath);
    this.fsMapping.setEntryMetaExtras({
      assetMeta: (entryPath: string) => this.getAssetMeta(entryPath, { silent: true }),
    });
    this.fsMapping.setProcessEntry(this.processFsEntry);

    this.fsMapping.setAfterUpdate((updateType, updatedPath, updatedEntry) => {
      this.sendFsUpdate();
      this.gcManifestResources();

      // Now notify related assets
      this.assets.fsUpdate(updateType, updatedPath, updatedEntry);
    });

    this.fsMapping.setOnDeleted((entryPath) => {
      this.assets.release(entryPath);
    });

    this.fsMapping.setOnRenamed((entry, oldEntryPath) => {
      this.assets.release(oldEntryPath);
    });

    await this.fsMapping.init(this.path, this.storagePath);
  }

  async unload() {
    this.log('Unloading...');

    this.apiClient.emit(projectApi.close);

    await this.assets.dispose();
    await this.fsMapping.deinit();
    this.eventDisposers.forEach((disposer) => disposer());

    this.gameServerService.stop();

    this.log('Unloaded');
  }

  private notifyProjectUpdated() {
    this.apiClient.emit(projectApi.update, this.getProjectData());
  }

  private sendFsUpdate = debounce(() => {
    if (this.fsMapping.hasUpdates()) {
      this.apiClient.emit(projectApi.fsUpdate, this.fsMapping.flushUpdates());
    }
  }, 100);

  private processFsEntry = async (entry: FilesystemEntry) => {
    if (entry.path.startsWith(this.storagePath)) {
      return;
    }

    if (isAssetMetaFile(entry.name)) {
      const assetPath = stripAssetMetaExt(entry.path);

      return this.fsMapping.forceEntryScan(assetPath);
    }

    this.assetManagers.fsEntry(entry);

    const existingAsset = this.assets.get(entry.path);
    if (existingAsset) {
      existingAsset.setEntry?.(entry);

      return;
    }

    this.assets.load(entry);
  }

  //#endregion lifecycle

  async setAssetsEnabled(resourceNames: string[], enabled: boolean) {
    resourceNames.forEach((resourceName) => {
      this.setAssetConfig({ assetPath: resourceName, config: { enabled } });
    });
  }

  getAssetConfig(assetPath: string): ProjectAssetBaseConfig {
    const assetRelativePath = this.fsService.relativePath(this.path, assetPath);

    return this.getManifest().assets[assetRelativePath] || { enabled: false };
  }

  @handlesClientEvent(projectApi.setResourceConfig)
  async setAssetConfig(request: ProjectSetAssetConfigRequest) {
    const assetRelativePath = this.fsService.relativePath(this.path, request.assetPath);

    const newConfig = {
      ...this.getAssetConfig(request.assetPath),
      ...request.config,
    };

    this.applyManifest((manifest) => {
      manifest.assets[assetRelativePath] = newConfig;
    });

    this.assetConfigChangeListeners[request.assetPath]?.(newConfig);

    this.apiClient.emit(assetApi.setConfig, [request.assetPath, newConfig]);

    this.refreshEnabledResources();
  }

  deleteAssetConfig(assetPath: string) {
    const assetRelativePath = this.fsService.relativePath(this.path, assetPath);

    this.applyManifest((manifest) => {
      delete manifest.assets[assetRelativePath];
    });
  }

  @handlesClientEvent(projectApi.setPathsState)
  setPathsState(pathsState: ProjectPathsState) {
    this.applyManifest((manifest) => {
      manifest.pathsState = pathsState;
    });
  }

  @handlesClientEvent(projectApi.setPathsStatePatch)
  setPathsStatePatch(patch: ProjectPathsState) {
    this.applyManifest((manifest) => {
      manifest.pathsState = {
        ...manifest.pathsState,
        ...patch,
      };
    });
  }

  @handlesClientEvent(projectApi.setServerUpdateChannel)
  setServerUpdateChannel(updateChannel: ServerUpdateChannel) {
    if (this.getManifest().serverUpdateChannel !== updateChannel) {
      this.gameServerService.stop();

      this.applyManifest((manifest) => {
        manifest.serverUpdateChannel = updateChannel;
      });
    }
  }

  @handlesClientEvent(projectApi.setSystemResources)
  setSystemResources(systemResources: SystemResource[]) {
    this.applyManifest((manifest) => {
      manifest.systemResources = systemResources;
    });

    this.refreshEnabledResources();
  }

  //#region assets

  getAssetMetaPath(assetPath: string): string {
    const assetName = this.fsService.basename(assetPath);
    const assetMetaFilename = assetName + assetMetaFileExt;

    return this.fsService.joinPath(this.fsService.dirname(assetPath), assetMetaFilename);;
  }

  async getAssetMeta(assetPath: string, options?: GetAssetMetaOptions): Promise<AssetMeta | null> {
    const assetMetaFilepath = this.getAssetMetaPath(assetPath);

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

    const assetMetaFilepath = this.getAssetMetaPath(assetPath);

    await this.fsService.writeFileJson(assetMetaFilepath, assetMeta);

    this.fsMapping.forceEntryScan(assetPath);
  }

  async hasAssetMeta(assetPath: string) {
    return !!(await this.fsService.statSafe(this.getAssetMetaPath(assetPath)));
  }

  protected getAssetImporterContribution(importerType: AssetImporterType): AssetImporterContribution {
    try {
      return this.assetImporterContributions.getTagged('importerType', importerType);
    } catch (e) {
      throw new Error(`No asset importer contribution of type ${importerType}`);
    }
  }

  @handlesClientEvent(assetApi.import)
  async importAsset(request: AssetImportRequest) {
    this.log('Importing asset', request);

    return this.getAssetImporterContribution(request.importerType).importAsset(request);
  }

  @handlesClientEvent(assetApi.create)
  async createAsset(request: AssetCreateRequest) {
    this.log('Creating asset', request);

    return this.assetManagers.get(request.assetType).createAsset(request);
  }

  @handlesClientEvent(assetApi.rename)
  async renameAsset(request: AssetRenameRequest) {
    this.log('Renaming asset', request);

    // Applying changes to project manifest first
    const { newAssetName, assetPath } = request;
    const newAssetPath = this.fsService.joinPath(this.fsService.dirname(assetPath), newAssetName);

    await this.assets.release(assetPath);

    const assetConfig = this.getAssetConfig(assetPath);
    if (assetConfig) {
      this.deleteAssetConfig(assetPath);

      this.setAssetConfig({
        assetPath: newAssetPath,
        config: assetConfig,
      });
    }


    const promises = [
      this.fsService.rename(assetPath, newAssetPath),
    ];

    const oldAssetMetaPath = this.getAssetMetaPath(assetPath);
    const newAssetMetaPath = this.getAssetMetaPath(newAssetPath);

    if (await this.fsService.statSafe(oldAssetMetaPath)) {
      promises.push(
        this.fsService.rename(oldAssetMetaPath, newAssetMetaPath),
      );
    }

    await Promise.all(promises);
  }

  @handlesClientCallbackEvent(assetApi.delete)
  async deleteAsset(request: AssetDeleteRequest): Promise<AssetDeleteResponse> {
    this.log('Deleting asset', request);

    const { assetPath } = request;
    const assetMetaPath = this.getAssetMetaPath(assetPath);
    const hasAssetMeta = !!(await this.fsService.statSafe(assetMetaPath));

    const assetStat = await this.fsService.statSafe(assetPath);
    if (!assetStat) {
      return AssetDeleteResponse.Ok;
    }

    await this.assets.release(assetPath);

    try {
      if (request.hardDelete) {
        await this.fsService.rimraf(assetPath);
        if (hasAssetMeta) {
          await this.fsService.rimraf(assetMetaPath);
        }
      } else {
        try {
          await this.fsService.moveToTrashBin(assetPath);
          if (hasAssetMeta) {
            await this.fsService.moveToTrashBin(assetMetaPath);
          }
        } catch (e) {
          this.logService.log(`Failed to recycle asset: ${e.toString()}`);
          this.fsMapping.forceEntryScan(assetPath);
          return AssetDeleteResponse.FailedToRecycle;
        }
      }

      return AssetDeleteResponse.Ok;
    } catch (e) {
      this.fsMapping.forceEntryScan(assetPath);
      throw e;
    } finally {
      if (assetStat.isDirectory()) {
        this.apiClient.emit(projectApi.freePendingFolderDeletion, assetPath);
      }
    }
  }

  //#endregion assets

  //#region fs-methods
  // Directories methods
  @handlesClientEvent(projectApi.createDirectory)
  async createDirectory({ directoryName, directoryPath }: ProjectCreateDirectoryRequest) {
    const directoryFullPath = this.fsService.joinPath(directoryPath, directoryName);

    await this.fsService.mkdirp(directoryFullPath);
  }

  @handlesClientCallbackEvent(projectApi.deleteDirectory)
  async deleteDirectory({ directoryPath, hardDelete }: DeleteDirectoryRequest): Promise<DeleteDirectoryResponse> {
    if (!(await this.fsService.statSafe(directoryPath))) {
      return DeleteDirectoryResponse.Ok;
    }

    await this.assets.release(directoryPath);

    try {
      if (hardDelete) {
        await this.fsService.rimraf(directoryPath);
      } else {
        try {
          await this.fsService.moveToTrashBin(directoryPath);
        } catch (e) {
          this.logService.log(`Failed to recycle directory: ${e.toString()}`);
          this.fsMapping.forceEntryScan(directoryPath);
          return DeleteDirectoryResponse.FailedToRecycle;
        }
      }

      return DeleteDirectoryResponse.Ok;
    } catch (e) {
      this.fsMapping.forceEntryScan(directoryPath);
      throw e;
    } finally {
      this.apiClient.emit(projectApi.freePendingFolderDeletion, directoryPath);
    }
  }

  @handlesClientEvent(projectApi.renameDirectory)
  async renameDirectory({ directoryPath, newDirectoryName }: ProjectRenameDirectoryRequest) {
    const newDirectoryPath = this.fsService.joinPath(this.fsService.dirname(directoryPath), newDirectoryName);

    await this.assets.release(directoryPath);
    await this.fsService.rename(directoryPath, newDirectoryPath);
  }
  // /Directory methods

  // Files methods
  @handlesClientEvent(projectApi.createFile)
  async createFile({ filePath, fileName }: ProjectCreateFileRequest) {
    const fileFullPath = this.fsService.joinPath(filePath, fileName);

    await this.fsService.writeFile(fileFullPath, '');
  }

  @handlesClientCallbackEvent(projectApi.deleteFile)
  async deleteFile({ filePath, hardDelete }: DeleteFileRequest): Promise<DeleteFileResponse> {
    if (!(await this.fsService.statSafe(filePath))) {
      return DeleteFileResponse.Ok;
    }

    await this.assets.release(filePath);

    if (hardDelete) {
      await this.fsService.rimraf(filePath);
    } else {
      try {
        await this.fsService.moveToTrashBin(filePath);
      } catch (e) {
        this.logService.log(`Failed to recycle file: ${e.toString()}`);
        this.fsMapping.forceEntryScan(filePath);
        return DeleteFileResponse.FailedToRecycle;
      }
    }

    return DeleteFileResponse.Ok;
  }

  @handlesClientEvent(projectApi.renameFile)
  async renameFile({ filePath, newFileName }: ProjectRenameFileRequest) {
    const newFilePath = this.fsService.joinPath(this.fsService.dirname(filePath), newFileName);

    await this.assets.release(filePath);
    await this.fsService.rename(filePath, newFilePath);
  }
  // /Files methods

  // FS methods
  @handlesClientEvent(projectApi.moveEntry)
  async moveEntry(request: MoveEntryRequest) {
    const { sourcePath, targetPath } = request;

    const newPath = this.fsService.joinPath(targetPath, this.fsService.basename(sourcePath));

    if (newPath === sourcePath) {
      return;
    }

    await this.assets.release(sourcePath);
    await this.fsService.rename(sourcePath, newPath);
  }

  @handlesClientEvent(projectApi.copyEntry)
  async copyEntry(request: CopyEntryRequest) {
    const sourceDirName = this.fsService.dirname(request.sourcePath);

    if (sourceDirName === request.targetPath) {
      return;
    }

    this.fsService.copy(request.sourcePath, request.targetPath);
  }

  @handlesClientEvent(projectApi.copyEntries)
  async copyEntries(request: CopyEntriesRequest) {
    if (!request.sourcePaths.length) {
      return;
    }

    await Promise.all(request.sourcePaths.map((sourcePath) => {
      return this.fsService.copy(sourcePath, request.targetPath);
    }));
  }
  // /FS methods
  //#endregion fs-methods

  @handlesClientEvent(projectApi.startServer)
  async startServer(request: ProjectStartServerRequest = {}) {
    // If we have system resources enabled and they're unavailable - no server, rip
    if (this.getManifest().systemResources.length > 0) {
      const systemResourcesAvailable = await this.systemResourcesService.getAvailablePromise();
      if (!systemResourcesAvailable) {
        this.notificationService.error('System resources unavailable, unable to start server');
        return;
      }
    }

    const serverStartRequest: ServerStartRequest = {
      fxserverCwd: this.fxserverCwd,
      updateChannel: this.manifestMapping.get().serverUpdateChannel,
      ...request,
    };

    this.refreshEnabledResources();
    this.gameServerService.start(serverStartRequest);
  }

  @handlesClientEvent(projectApi.stopServer)
  stopServer() {
    this.gameServerService.stop();
  }

  private gcManifestResources = debounce(() => {
    this.log('Cleaning up manifest resources');

    // FIXME: might be a good idea to call this only on load
    this.applyManifest((manifest) => {
      for (const assetRelativePath of Object.keys(manifest.assets)) {
        const assetPath = this.fsService.joinPath(this.path, assetRelativePath);

        if (!this.hasAsset(assetPath)) {
          delete manifest.assets[assetRelativePath];
        }
      }

      this.refreshEnabledResources();
    });
  }, 100);

  private refreshEnabledResources() {
    if (this.worldEditorService.isRunning()) {
      return;
    }

    const enabledSystemResources = this.getManifest().systemResources;

    const resourceDescriptors: ServerResourceDescriptor[] = this.systemResourcesService.getResourceDescriptors(enabledSystemResources).slice();

    for (const asset of this.getEnabledAssets()) {
      if (asset.getResourceDescriptor) {
        const descriptor = asset.getResourceDescriptor();

        // Only if it doesn't conflict with enabled system resource name
        if (enabledSystemResources.indexOf(descriptor.name as SystemResource) === -1) {
          resourceDescriptors.push();
        }
      }
    }

    this.gameServerService.setResources(resourceDescriptors);
  }
}
