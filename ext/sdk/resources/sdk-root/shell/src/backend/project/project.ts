import { inject, injectable, named } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from 'backend/api/api-decorators';
import { fxdkAssetFilename, fxdkProjectFilename } from 'backend/constants';
import { ExplorerService } from 'backend/explorer/explorer-service';
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
  FilesystemEntry,
  FilesystemEntryMap,
  ProjectData,
  ProjectManifest,
  ProjectManifestResource,
  ProjectPathsState,
  ProjectResources,
  ServerUpdateChannel,
  serverUpdateChannels,
} from "shared/api.types";
import { debounce, getResourceConfig } from 'shared/utils';
import { ContributionProvider } from 'backend/contribution-provider';
import { AssetContribution, AssetInterface } from './asset/asset-contribution';
import { GameServerService } from 'backend/game-server/game-server-service';
import { FsJsonFileMapping, FsJsonFileMappingOptions } from 'backend/fs/fs-json-file-mapping';
import { FsMapping } from 'backend/fs/fs-mapping';
import { ChangeAwareContainer } from 'backend/change-aware-container';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { TheiaService } from 'backend/theia/theia-service';

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

  @inject(ContributionProvider) @named(AssetContribution)
  protected readonly assetContributions: ContributionProvider<AssetContribution>;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(TheiaService)
  protected readonly theiaService: TheiaService;

  @inject(FsMapping)
  private fsMapping: FsMapping;

  private path: string;
  private assets: Map<string, AssetInterface> = new Map();
  private manifestMapping: FsJsonFileMapping<ProjectManifest>;
  private manifestPath: string;
  private storagePath: string;
  private shadowPath: string;
  private resources = new ChangeAwareContainer<ProjectResources>({}, (resources: ProjectResources) => {
    if (this.ready) {
      this.apiClient.emit(projectApi.resourcesUpdate, resources);
    }
  });

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

  getResources(): ProjectResources {
    return this.resources.get();
  }

  getProjectData(): ProjectData {
    return {
      path: this.getPath(),
      manifest: this.getManifest(),

      fs: this.getFs(),
      resources: this.getResources(),
    };
  }

  getResourceConfig(resourceName: string): ProjectManifestResource {
    return getResourceConfig(this.getManifest(), resourceName);
  }

  getEnabledResourcesPaths(): string[] {
    return Object.values(this.getResources()).filter((resource) => resource.enabled).map((resource) => resource.path);
  }

  log(msg: string, ...args) {
    this.logService.log(`[ProjectInstance: ${this.path}]`, msg, ...args);
  }

  //#region lifecycle

  async create(request: ProjectCreateRequest): Promise<Project> {
    this.logService.log('Creating project', request);

    const creatingTask = this.taskReporterService.createNamed(projectCreatingTaskName, `Creating project ${request.projectName}`);

    try {
      this.path = this.fsService.joinPath(request.projectPath, request.projectName);
      this.storagePath = this.fsService.joinPath(this.path, '.fxdk');
      this.shadowPath = this.fsService.joinPath(this.storagePath, 'shadowRoot');

      const projectManifestPath = this.fsService.joinPath(this.path, fxdkProjectFilename);
      const projectManifest: ProjectManifest = {
        name: request.projectName,
        serverUpdateChannel: serverUpdateChannels.recommended,
        createdAt: new Date().toISOString(),
        updatedAt: new Date().toISOString(),
        resources: {},
        pathsState: {},
      };

      // It will create this.path as well
      await this.fsService.mkdirp(this.shadowPath)

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

    return this.load();
  }

  async load(path?: string): Promise<Project> {
    if (path) {
      this.path = path;
    }

    const loadTask = this.taskReporterService.createNamed(projectLoadingTaskName, `Loading project ${path}`);

    this.manifestPath = this.fsService.joinPath(this.path, fxdkProjectFilename);
    this.storagePath = this.fsService.joinPath(this.path, '.fxdk');
    this.shadowPath = this.fsService.joinPath(this.storagePath, 'shadowRoot');

    try {
      this.log('loading project...');

      loadTask.setText('Loading manifest...');
      await this.initManifest();

      loadTask.setText('Loading project files...');
      await this.initFs();

      this.setGameServerServiceEnabledResources();

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
        pathsState: {},
        resources: {},
        serverUpdateChannel: serverUpdateChannels.recommended,
      },
      onApply: () => this.notifyProjectUpdated(),
      beforeApply: (snapshot) => snapshot.updatedAt = new Date().toISOString(),
    };

    this.manifestMapping = await this.fsService.createJsonFileMapping<ProjectManifest>(this.manifestPath, options);
  }

  private async initFs() {
    this.log('Initializing project fs...');

    this.fsMapping.setShouldProcessUpdate((type, path) => path !== this.manifestPath);
    this.fsMapping.setEntryMetaExtras({
      assetMeta: async (entryPath: string) => {
        return this.getAssetMeta(entryPath, { silent: true });
      },
    });
    this.fsMapping.setProcessEntry(this.processFsEntry);

    this.fsMapping.setAfterUpdate((updateType, updatedPath, updatedEntry) => {
      this.sendFsUpdate();
      this.gcManifestResources();

      // Now notify related assets
      for (const asset of this.findAssetsForPath(updatedPath)) {
        asset.onFsEvent(updateType, updatedEntry);
      }
    });

    this.fsMapping.setOnUnlink((entryPath) => {
      this.assets.delete(entryPath);
    });

    this.fsMapping.setOnUnlinkDir((entryPath) => {
      this.resources.apply((resources) => {
        delete resources[this.fsService.basename(entryPath)];
      });
      this.assets.delete(entryPath);
    });

    await this.fsMapping.init(this.path, this.storagePath);
  }

  async unload() {
    this.log('Unloading...');

    this.apiClient.emit(projectApi.close);

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

    if (entry.meta.isResource) {
      this.resources.apply((resources) => {
        resources[entry.name] = {
          ...this.getResourceConfig(entry.name),
          path: entry.path,
          running: false,
        };
      });
    }

    const existingAsset = this.assets.get(entry.path);
    if (existingAsset) {
      existingAsset.setEntry?.(entry);

      return;
    }

    this.assetContributions.getAll().forEach(async (contribution) => {
      const asset = await contribution.loadAsset(this, entry);
      if (asset) {
        this.assets.set(entry.path, asset);
      }
    });
  }

  //#endregion lifecycle

  async setResourcesEnabled(resourceNames: string[], enabled: boolean) {
    resourceNames.forEach((resourceName) => {
      this.setResourceConfig({ resourceName, config: { enabled } });
    });
  }

  @handlesClientEvent(projectApi.setResourceConfig)
  async setResourceConfig({ resourceName, config }: ProjectSetResourceConfigRequest) {
    const newConfig = {
      ...this.getResourceConfig(resourceName),
      ...config,
    };

    this.applyManifest((manifest) => {
      manifest.resources[resourceName] = newConfig;
    });

    if (this.getResources()[resourceName]) {
      this.resources.apply((resources) => {
        resources[resourceName] = {
          ...resources[resourceName],
          ...newConfig,
        };
      });
    }

    this.setGameServerServiceEnabledResources();
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
    if (this.getManifest().serverUpdateChannel !== updateChannel) {
      this.gameServerService.stop();

      this.applyManifest((manifest) => {
        manifest.serverUpdateChannel = updateChannel;
      });
    }
  }

  //#region assets

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

    this.fsMapping.syncEntry(assetPath);
  }

  protected getNamedAssetContribution(managerName: string): AssetContribution {
    try {
      return this.assetContributions.getTagged('managerName', managerName);
    } catch (e) {
      throw new Error(`No asset manager of type ${managerName}`);
    }
  }

  @handlesClientEvent(assetApi.create)
  async createAsset(request: AssetCreateRequest) {
    this.log('Creating asset', request);

    if (request.managerName) {
      const contribution = this.getNamedAssetContribution(request.managerName);

      if (contribution.capabilities[request.action]) {
        switch (request.action) {
          case 'create': {
            if (!contribution.createAsset) {
              throw new Error(`Asset contribution ${contribution.name} have create capability but does not implement createAsset method`);
            }
            return contribution.createAsset(this, request);
          }
          case 'import': {
            if (!contribution.importAsset) {
              throw new Error(`Asset contribution ${contribution.name} have import capability but does not implement importAsset method`);
            }
            return contribution.importAsset(this, request);
          }
        }
      }

      throw new Error(`Asset contribution ${request.managerName} has no ${request.action} capability`);
    }

    throw new Error('Invalid asset create request managerName must be specified');
  }

  @handlesClientEvent(assetApi.rename)
  async renameAsset(request: AssetRenameRequest) {
    this.log('Renaming asset', request);

    // Applying changes to project manifest first
    const { newAssetName, assetPath } = request;
    const oldAssetName = this.fsService.basename(request.assetPath);

    const resourceConfig = this.getManifest().resources[oldAssetName];
    if (resourceConfig) {
      this.applyManifest((manifest) => {
        manifest.resources[newAssetName] = {
          ...resourceConfig,
          name: newAssetName,
        };
      });
    }

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

    const { assetPath } = request;

    await Promise.all([
      this.fsService.rimraf(assetPath),
      this.fsService.rimraf(this.getPathInShadow(assetPath)),
    ]);
  }

  //#endregion assets

  //#region fs-methods
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
    const { sourcePath, targetPath } = request;

    const newPath = this.fsService.joinPath(targetPath, this.fsService.basename(sourcePath));

    if (newPath === sourcePath) {
      return;
    }

    await this.fsService.rename(sourcePath, newPath);
  }

  @handlesClientEvent(projectApi.copyEntry)
  async copyEntry(request: CopyEntryRequest) {
    this.fsService.copy(request.sourcePath, request.targetPath);
  }
  // /FS methods
  //#endregion fs-methods

  private gcManifestResources = debounce(() => {
    this.log('Cleaning up manifest resources');

    this.applyManifest((manifest) => {
      const manifestResources = {};

      Object.values(this.getResources())
        .forEach(({ name }) => {
          manifestResources[name] = manifest.resources[name];
        });

      if (Object.keys(manifest.resources).length !== Object.keys(manifestResources).length) {
        manifest.resources = manifestResources;

        this.setGameServerServiceEnabledResources();
      }
    });
  }, 100);

  private *findAssetsForPath(entryPath: string): IterableIterator<AssetInterface> {
    for (const [assetPath, asset] of this.assets.entries()) {
      if (entryPath.indexOf(assetPath) > -1) {
        yield asset;
      }
    }
  }

  private setGameServerServiceEnabledResources() {
    this.log('Setting resources as enabled', this.getResources(), this.getEnabledResourcesPaths());

    this.gameServerService.setEnabledResources({
      projectPath: this.path,
      enabledResourcesPaths: this.getEnabledResourcesPaths(),
    });
  }
}
