import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientCallbackEvent, handlesClientEvent } from 'backend/api/api-decorators';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { assetApi, projectApi } from 'shared/api.events';
import { APIRQ } from 'shared/api.requests';
import { FilesystemEntryMap, ServerUpdateChannel, serverUpdateChannels } from "shared/api.types";
import { debounce } from 'shared/utils';
import { GameServerService } from 'backend/game-server/game-server-service';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { TheiaService } from 'backend/theia/theia-service';
import { getAssetsPriorityQueue } from './project-utils';
import { AssetMeta } from 'shared/asset.types';
import { AssetInterface } from '../../assets/core/asset-interface';
import { ProjectAssetBaseConfig, ProjectData, ProjectManifest, ProjectOpenData, ProjectPathsState } from 'shared/project.types';
import { ProjectUpgrade } from './project-upgrade';
import { ServerResourceDescriptor, ServerStartRequest } from 'backend/game-server/game-server-runtime';
import { DisposableObject } from 'backend/disposable-container';
import { WorldEditorService } from 'backend/world-editor/world-editor-service';
import { SystemResource } from 'backend/system-resources/system-resources-constants';
import { SystemResourcesService } from 'backend/system-resources/system-resources-service';
import { DEFAULT_PROJECT_SYSTEM_RESOURCES } from './project-constants';
import { AssetConfigChangedListener, ProjectRuntime } from './project-runtime/project-runtime';
import { ProjectState } from './project-runtime/project-state-runtime';
import { GetAssetMetaOptions } from './project-runtime/project-assets-runtime';
import { ContainerAccess } from 'backend/container-access';

@injectable()
export class Project implements ApiContribution {
  getId() {
    return `ProjectService(${this.getPath() || '#loading#'})`;
  }

  eventDisposers: Function[] = [];

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(TheiaService)
  protected readonly theiaService: TheiaService;

  @inject(ProjectUpgrade)
  protected readonly projectUpgrade: ProjectUpgrade;

  @inject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @inject(SystemResourcesService)
  protected readonly systemResourcesService: SystemResourcesService;

  private rt: ProjectRuntime;

  onAssetConfigChanged(assetPath: string, listener: AssetConfigChangedListener): DisposableObject {
    return this.rt.onAssetConfigChanged(assetPath, listener);
  }

  applyManifest(fn: (manifest: ProjectManifest) => void) {
    this.rt.applyManifest(fn);
  }

  getManifest(): ProjectManifest {
    return this.rt.getManifest();
  }

  getName(): string {
    return this.rt.getManifest().name;
  }

  getPath(): string {
    return this.rt.state.path;
  }

  getStoragePath(): string {
    return this.rt.state.storagePath;
  }

  getFs(): FilesystemEntryMap {
    return this.rt.getFSMap();
  }

  getAsset(assetPath: string): AssetInterface {
    return this.rt.assets.get(assetPath);
  }

  releaseAsset(assetPath: string) {
    return this.rt.assets.release(assetPath);
  }

  refreshAssetInfo(assetPath: string) {
    return this.rt.assets.refreshInfo(assetPath);
  }

  forceFSScan(entryPath: string) {
    this.rt.forceFSScan(entryPath);
  }

  getProjectData(): ProjectData {
    const assets = {};
    const assetTypes = {};
    const assetDefs = {};

    for (const assetPath of this.rt.assets.getAllPaths()) {
      const asset = this.getAsset(assetPath);

      assets[assetPath] = this.getAssetConfig(assetPath);
      assetTypes[assetPath] = asset?.type;
      assetDefs[assetPath] = asset?.getDefinition?.() ?? {};
    }

    return {
      assets,
      assetTypes,
      assetDefs,
      fs: this.getFs(),
      path: this.getPath(),
      manifest: this.getManifest(),
    };
  }

  getProjectOpenData(): ProjectOpenData {
    return {
      project: this.getProjectData(),
      pathsState: this.rt.pathsState.get(),
    };
  }

  getEnabledAssets(): AssetInterface[] {
    const enabledAssets: AssetInterface[] = [];

    for (const [assetRelativePath, assetConfig] of Object.entries(this.getManifest().assets)) {
      const assetPath = this.rt.state.getPathInProject(assetRelativePath);
      const asset = this.getAsset(assetPath);

      if (asset && assetConfig.enabled) {
        enabledAssets.push(asset);
      }
    }

    return enabledAssets;
  }

  getFxserverCwd(): string {
    return this.rt.state.fxserverCwd;
  }

  getUpdateChannel(): ServerUpdateChannel {
    return this.rt.getManifest().serverUpdateChannel;
  }

  hasAsset(assetPath: string): boolean {
    return this.rt.assets.has(assetPath);
  }

  isAsset<T extends new () => AssetInterface>(assetPath: string, ctor: T): boolean {
    const asset = this.rt.assets.get(assetPath);

    return asset && (asset instanceof ctor);
  }

  enterState(state: ProjectState) {
    this.rt.state.setState(state);
  }

  isInDevState(): boolean {
    return this.rt.state.getState() === ProjectState.Development;
  }

  async runBuildCommands() {
    const priorityQueue = getAssetsPriorityQueue(this.rt.assets.getAllPaths());

    for (const assetPathsBatch of priorityQueue) {
      await Promise.all(assetPathsBatch.map((assetPath) => {
        const asset = this.getAsset(assetPath);
        if (!asset?.build) {
          return;
        }

        return asset.build();
      }));
    }
  }

  //#region lifecycle

  async create(request: APIRQ.ProjectCreate): Promise<Project> {
    this.logService.log('Creating project', request);

    const creatingTask = this.taskReporterService.createNamed(projectCreatingTaskName, `Creating project ${request.projectName}`);

    this.rt = this.containerAccess.resolve(ProjectRuntime);
    await this.rt.initState(this.fsService.joinPath(request.projectPath, request.projectName));

    const projectManifest: ProjectManifest = {
      name: request.projectName,
      assets: {},
      createdAt: new Date().toISOString(),
      serverUpdateChannel: serverUpdateChannels.recommended,
      systemResources: DEFAULT_PROJECT_SYSTEM_RESOURCES,
      variables: {},
    };

    try {
      await Promise.all([
        // Write project manifest
        this.fsService.writeFileJson(this.rt.state.manifestPath, projectManifest),

        // Write paths state
        this.fsService.writeFileJson(this.rt.state.pathsStatePath, {}),

        // Write theia-personality settings
        this.theiaService.createDefaultProjectSettings(this.rt.state.storagePath),
      ]);

      this.logService.log('Done creating project', request);
    } catch (e) {
      throw e;
    } finally {
      creatingTask.done();
    }

    return this.load(false);
  }

  async open(projectPath: string): Promise<Project> {
    this.rt = this.containerAccess.resolve(ProjectRuntime);
    await this.rt.initState(this.fsService.resolvePath(projectPath));

    return this.load(true);
  }

  private async load(runUpgradeRoutines: boolean): Promise<Project> {
    const loadTask = this.taskReporterService.createNamed(projectLoadingTaskName, `Loading project ${this.rt.state.path}`);

    try {
      if (runUpgradeRoutines) {
        await this.projectUpgrade.maybeUpgradeProject({
          task: loadTask,
          projectPath: this.rt.state.path,
          manifestPath: this.rt.state.manifestPath,
          storagePath: this.rt.state.storagePath,
        });
      }

      this.rt.manifestUpdated.addListener(() => {
        this.refreshEnabledResources();
      });

      this.rt.onAnyAssetConfigChanged(() => {
        this.refreshEnabledResources();
      });

      await this.rt.init(loadTask);

      this.gcManifestResources();

      this.refreshEnabledResources();
    } catch (e) {
      throw e;
    } finally {
      loadTask.done();
    }

    return this;
  }

  async unload() {
    this.logService.log(`Unloading project(${this.getName()})...`);

    await this.gameServerService.stop();
    await this.worldEditorService.stop();

    this.apiClient.emit(projectApi.close);

    await this.rt.dispose();

    this.eventDisposers.forEach((disposer) => disposer());

    this.logService.log(`Unloaded project(${this.getName()})`);
  }

  //#endregion lifecycle
  @handlesClientEvent(projectApi.setPathsStatePatch)
  setPathsStatePatch(patch: ProjectPathsState) {
    this.rt.pathsState.applyPartial(patch);
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

  @handlesClientEvent(projectApi.setVariable)
  setVariable({ key, value }: { key: string, value: string }) {
    this.applyManifest((manifest) => {
      if (!manifest.variables) {
        manifest.variables = {};
      }

      manifest.variables[key] = value;
    });

    this.refreshVariables();
  }

  //#region assets

  getAssetConfig(assetPath: string): ProjectAssetBaseConfig {
    return this.rt.assets.getConfig(assetPath);
  }

  @handlesClientEvent(projectApi.setAssetConfig)
  async setAssetConfig(request: APIRQ.ProjectSetAssetConfig) {
    return this.rt.assets.setConfig(request);
  }

  async getAssetMeta(assetPath: string, options?: GetAssetMetaOptions): Promise<AssetMeta | null> {
    return this.rt.assets.getMeta(assetPath, options);
  }

  async setAssetMeta(assetPath: string, assetMeta: AssetMeta) {
    return this.rt.assets.setMeta(assetPath, assetMeta);
  }

  @handlesClientEvent(assetApi.import)
  async importAsset(request: APIRQ.AssetImport) {
    return this.rt.assets.import(request);
  }

  @handlesClientEvent(assetApi.create)
  async createAsset(request: APIRQ.AssetCreate) {
    return this.rt.assets.create(request);
  }

  //#endregion assets

  //#region fs-methods
  @handlesClientEvent(projectApi.createDirectory)
  async createDirectory(request: APIRQ.ProjectCreateDirectory) {
    return this.rt.fs.createDirectory(request);
  }

  @handlesClientEvent(projectApi.createFile)
  async createFile(request: APIRQ.ProjectCreateFile) {
    return this.rt.fs.createFile(request);
  }

  @handlesClientEvent(projectApi.renameEntry)
  async renameEntry(request: APIRQ.RenameEntry) {
    if (this.hasAsset(request.entryPath)) {
      return this.rt.assets.rename(request);
    }

    return this.rt.fs.renameEntry(request);
  }

  @handlesClientCallbackEvent(projectApi.deleteEntry)
  async deleteEntry(request: APIRQ.DeleteEntry): Promise<APIRQ.DeleteEntryResponse> {
    if (this.hasAsset(request.entryPath)) {
      return this.rt.assets.delete(request);
    }

    return this.rt.fs.deleteEntry(request);
  }

  @handlesClientEvent(projectApi.moveEntry)
  async moveEntry(request: APIRQ.MoveEntry) {
    if (this.hasAsset(request.sourcePath)) {
      return this.rt.assets.move(request);
    }

    return this.rt.fs.moveEntry(request);
  }

  @handlesClientEvent(projectApi.copyEntry)
  async copyEntry(request: APIRQ.CopyEntry) {
    return this.rt.fs.copyEntry(request);
  }

  @handlesClientEvent(projectApi.copyEntries)
  async copyEntries(request: APIRQ.CopyEntries) {
    return this.rt.fs.copyEntries(request);
  }
  //#endregion fs-methods

  @handlesClientEvent(projectApi.startServer)
  async startServer(request: APIRQ.ProjectStartServer = {}) {
    // If we have system resources enabled and they're unavailable - no server, rip
    if (this.getManifest().systemResources.length > 0) {
      const systemResourcesAvailable = await this.systemResourcesService.getAvailablePromise();
      if (!systemResourcesAvailable) {
        this.notificationService.error('System resources unavailable, unable to start server');
        return;
      }
    }

    const serverStartRequest: ServerStartRequest = {
      fxserverCwd: this.rt.state.fxserverCwd,
      updateChannel: this.getUpdateChannel(),
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
    this.logService.log(`Cleaning up manifest resources project(${this.getName()})`);

    // FIXME: might be a good idea to call this only on load
    this.applyManifest((manifest) => {
      for (const assetRelativePath of Object.keys(manifest.assets)) {
        const assetPath = this.rt.state.getPathInProject(assetRelativePath);

        if (!this.hasAsset(assetPath)) {
          delete manifest.assets[assetRelativePath];
        }
      }

      this.refreshEnabledResources();
    });
  }, 1000);

  private refreshVariables() {
    if (this.worldEditorService.isRunning()) {
      return;
    }

    for (const cmd of this.getAssetsConvarCommands()) {
      this.gameServerService.sendCommand(cmd);
    }
  }

  *getAssetsConvarCommands() {
    const convarMap = this.getEnabledAssets()
      .map((asset) => asset.getDefinition?.()?.convarCategories)
      .filter(Boolean)
      .filter((convarCategories) => Array.isArray(convarCategories))
      .map((convarCategories) => (convarCategories as any).map(([, [, setting]]) => setting))
      .flat(2);

    for (const setting of convarMap) {
      const [title, id, type, def, ...more]: [string, string, string, any, any[]] = setting;
      const value = this.getManifest().variables?.[id] ?? mapDefault(def);
      let setType = 'set';
      let finalId = id;

      if (id[0] == '#') {
        setType = 'sets';
        finalId = id.substring(1);
      } else if (id[0] == '$') {
        setType = 'setr';
        finalId = id.substring(1);
      }

      yield `${setType} "${escapeCmd(finalId)}" "${escapeCmd(value)}"`;
    }

    function escapeCmd(value: string) {
      return value.replace(/\\/g, "\\\\").replace(/"/g, "\\\"");
    }

    function mapDefault(def: any) {
      if (Array.isArray(def)) {
        return def[0][1] + '';
      }

      return def + '';
    }
  }

  private refreshEnabledResources() {
    if (this.worldEditorService.isRunning()) {
      return;
    }

    const enabledSystemResources = this.getManifest().systemResources;

    const resourceDescriptors: ServerResourceDescriptor[] = this.systemResourcesService.getResourceDescriptors(enabledSystemResources).slice();

    for (const asset of this.getEnabledAssets()) {
      const descriptor = asset.getResourceDescriptor?.();
      if (descriptor) {
        // Only if it doesn't conflict with enabled system resource name
        if (enabledSystemResources.indexOf(descriptor.name as SystemResource) === -1) {
          resourceDescriptors.push(descriptor);
        }
      }
    }

    this.gameServerService.setResources(resourceDescriptors);

    // also update variables as resource config may have changed
    this.refreshVariables();
  }
}
