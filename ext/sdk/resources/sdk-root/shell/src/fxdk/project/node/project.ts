import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { ApiContribution } from "backend/api/api.extensions";
import { handlesClientCallbackEvent, handlesClientEvent } from 'backend/api/api-decorators';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { APIRQ } from 'shared/api.requests';
import { ServerUpdateChannel, serverUpdateChannels } from "shared/api.types";
import { debounce } from 'shared/utils';
import { GameServerService } from 'backend/game-server/game-server-service';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { getAssetsPriorityQueue } from './project-utils';
import { AssetMeta } from 'shared/asset.types';
import { ProjectAssetBaseConfig, ProjectManifest, ProjectOpenData } from 'shared/project.types';
import { ProjectUpgrade } from './project-upgrade';
import { ServerResourceDescriptor, ServerStartRequest, ServerVariableDescriptor } from 'backend/game-server/game-server-runtime';
import { dispose, Disposer, IDisposable } from 'fxdk/base/disposable';
import { WorldEditorService } from 'backend/world-editor/world-editor-service';
import { SystemResource } from 'backend/system-resources/system-resources-constants';
import { SystemResourcesService } from 'backend/system-resources/system-resources-service';
import { DEFAULT_PROJECT_SYSTEM_RESOURCES } from '../common/project.constants';
import { ProjectRuntime } from './runtime/project-runtime';
import { ProjectState } from './runtime/project-state-runtime';
import { ContainerAccess } from 'backend/container-access';
import { ProjectEvents } from './project-events';
import { concurrently } from 'utils/concurrently';
import { IAssetRuntime } from 'fxdk/project/node/asset/asset-runtime';
import { ProjectApi } from '../common/project.api';
import { ProjectPathsState } from '../common/project.types';

@injectable()
export class ProjectInstanceService implements ApiContribution {
  getId() {
    return `ProjectService(${this.getPath() || '#loading#'})`;
  }

  eventDisposers: IDisposable[] = [];

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

  @inject(ProjectUpgrade)
  protected readonly projectUpgrade: ProjectUpgrade;

  @inject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @inject(SystemResourcesService)
  protected readonly systemResourcesService: SystemResourcesService;

  private rt: ProjectRuntime;
  private toDispose = new Disposer();

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

  getAsset(assetPath: string): IAssetRuntime | undefined {
    return this.rt.assets.get(assetPath);
  }

  getProjectOpenData(): ProjectOpenData {
    return {
      rootFsEntry: this.rt.fs.getRootFsEntry(),
      path: this.getPath(),
      manifest: this.getManifest(),
      pathsState: this.rt.pathsState.get(),
    };
  }

  getEnabledAssets(): IAssetRuntime[] {
    const enabledAssets: IAssetRuntime[] = [];

    for (const [assetRelativePath, assetConfig] of Object.entries(this.getManifest().assets)) {
      if (!assetConfig.enabled) {
        continue;
      }

      const assetPath = this.rt.state.getPathInProject(assetRelativePath);
      const asset = this.getAsset(assetPath);

      if (asset) {
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

  async create(request: APIRQ.ProjectCreate): Promise<ProjectInstanceService> {
    this.logService.log('Creating project', request);

    const projectParentPath = this.fsService.resolvePath(request.projectPath);
    const projectParentPathStat = await this.fsService.statSafe(projectParentPath);

    if (!projectParentPathStat) {
      throw new Error(`The folder to create project in does not exist: ${projectParentPath}`);
    }

    const fullProjectPath = this.fsService.joinPath(projectParentPath, request.projectName);

    const creatingTask = this.taskReporterService.createNamed(projectCreatingTaskName, `Creating project ${request.projectName}`);

    this.rt = this.containerAccess.resolve(ProjectRuntime);
    await this.rt.initState(fullProjectPath);

    const projectManifest: ProjectManifest = {
      name: request.projectName,
      assets: {},
      createdAt: new Date().toISOString(),
      serverUpdateChannel: serverUpdateChannels.recommended,
      systemResources: DEFAULT_PROJECT_SYSTEM_RESOURCES,
      variables: {},
    };

    try {
      await concurrently(
        // Write project manifest
        this.fsService.writeFileJson(this.rt.state.manifestPath, projectManifest),

        // Write paths state
        this.fsService.writeFileJson(this.rt.state.pathsStatePath, {}),
      );

      this.logService.log('Done creating project', request);
    } catch (e) {
      throw e;
    } finally {
      creatingTask.done();
    }

    await ProjectEvents.Created.emit(this.rt.state);

    return this.load(false);
  }

  async open(projectPath: string): Promise<ProjectInstanceService> {
    this.rt = this.containerAccess.resolve(ProjectRuntime);
    await this.rt.initState(this.fsService.resolvePath(projectPath));

    await ProjectEvents.Opened.emit(this.rt.state);

    return this.load(true);
  }

  private async load(runUpgradeRoutines: boolean): Promise<ProjectInstanceService> {
    const loadTask = this.taskReporterService.createNamed(projectLoadingTaskName, `Loading project ${this.rt.state.path}`);

    await ProjectEvents.BeforeLoad.emit(this.rt.state);

    try {
      if (runUpgradeRoutines) {
        await this.projectUpgrade.maybeUpgradeProject({
          task: loadTask,
          projectPath: this.rt.state.path,
          manifestPath: this.rt.state.manifestPath,
          storagePath: this.rt.state.storagePath,
        });
      }

      this.toDispose.register(ProjectEvents.ManifestUpdated.addListener(() => {
        this.refreshEnabledResources();
      }));

      await this.rt.init(loadTask);

      this.gcManifestResources();

      this.refreshEnabledResources();

      await ProjectEvents.Loaded.emit(this.rt);
    } catch (e) {
      throw e;
    } finally {
      loadTask.done();
    }

    return this;
  }

  async unload() {
    this.logService.log(`Unloading project(${this.getName()})...`);

    await ProjectEvents.BeforeUnload.emit(this.rt);

    await concurrently(
      dispose(this.toDispose),
      dispose(this.rt),
    );

    this.eventDisposers.forEach(dispose);

    this.logService.log(`Unloaded project(${this.getName()})`);

    this.apiClient.emit(ProjectApi.LoaderEndpoints.close);
  }

  //#endregion lifecycle
  @handlesClientEvent(ProjectApi.FsEndpoints.setPathsStatePatch)
  setPathsStatePatch(patch: ProjectPathsState) {
    this.rt.pathsState.applyPartial(patch);
  }

  @handlesClientEvent(ProjectApi.ManifestEndpoints.setServerUpdateChannel)
  setServerUpdateChannel(updateChannel: ServerUpdateChannel) {
    if (this.getManifest().serverUpdateChannel !== updateChannel) {
      this.gameServerService.stop();

      this.applyManifest((manifest) => {
        manifest.serverUpdateChannel = updateChannel;
      });
    }
  }

  @handlesClientEvent(ProjectApi.ManifestEndpoints.setSystemResources)
  setSystemResources(systemResources: SystemResource[]) {
    this.applyManifest((manifest) => {
      manifest.systemResources = systemResources;
    });

    this.refreshEnabledResources();
  }

  @handlesClientEvent(ProjectApi.ManifestEndpoints.setVariable)
  setVariable(request: ProjectApi.ManifestRequests.SetVariable) {
    this.applyManifest((manifest) => {
      if (!manifest.variables) {
        manifest.variables = {};
      }

      manifest.variables[request.variable] = {
        setter: request.setter,
        value: request.value,
      };
    });

    this.refreshVariables();
  }

  @handlesClientEvent(ProjectApi.ManifestEndpoints.deleteVariable)
  deleteVariable(variable: string) {
    this.applyManifest((manifest) => {
      if (manifest.variables) {
        delete manifest.variables[variable];
      }
    });

    this.refreshVariables();
  }

  //#region assets
  getAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string): T {
    return this.rt.manifest.getAssetConfig<T>(assetPath, { enabled: false } as any);
  }

  @handlesClientEvent(ProjectApi.AssetEndpoints.setAssetConfig)
  setAssetConfigFromRequest(request: APIRQ.ProjectSetAssetConfig) {
    this.setAssetConfig(request.assetPath, request.config);
  }

  setAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string, config: Partial<T>) {
    return this.rt.manifest.setAssetConfig(assetPath, {
      ...this.getAssetConfig(assetPath),
      ...config,
    });
  }
  //#endregion assets

  //#region fs-methods
  setAssetMeta(entryPath: string, meta: AssetMeta) {
    this.rt.fs.setEntryMeta(entryPath, meta);
  }

  @handlesClientEvent(ProjectApi.FsEndpoints.createDirectory)
  async createDirectory(request: APIRQ.ProjectCreateDirectory) {
    return this.rt.fs.createDirectory(request);
  }

  @handlesClientCallbackEvent(ProjectApi.FsEndpoints.createFile)
  async createFile(request: APIRQ.ProjectCreateFile) {
    return this.rt.fs.createFile(request);
  }

  @handlesClientEvent(ProjectApi.FsEndpoints.renameEntry)
  async renameEntry(request: APIRQ.RenameEntry) {
    return this.rt.fs.renameEntry(request);
  }

  @handlesClientCallbackEvent(ProjectApi.FsEndpoints.deleteEntry)
  async deleteEntry(request: APIRQ.DeleteEntry): Promise<APIRQ.DeleteEntryResponse> {
    return this.rt.fs.deleteEntry(request);
  }

  @handlesClientEvent(ProjectApi.FsEndpoints.moveEntry)
  async moveEntry(request: APIRQ.MoveEntry) {
    return this.rt.fs.moveEntry(request);
  }

  @handlesClientEvent(ProjectApi.FsEndpoints.copyEntry)
  async copyEntry(request: APIRQ.CopyEntry) {
    return this.rt.fs.copyEntry(request);
  }

  @handlesClientEvent(ProjectApi.FsEndpoints.copyEntries)
  async copyEntries(request: APIRQ.CopyEntries) {
    return this.rt.fs.copyEntries(request);
  }
  //#endregion fs-methods

  @handlesClientEvent(ProjectApi.ServerEndpoints.start)
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

  @handlesClientEvent(ProjectApi.ServerEndpoints.stop)
  stopServer() {
    this.gameServerService.stop();
  }

  private gcManifestResources = debounce(() => {
    this.logService.log(`Cleaning up manifest resources project(${this.getName()})`);

    // FIXME: might be a good idea to call this only on load
    // this.applyManifest((manifest) => {
    //   for (const assetRelativePath of Object.keys(manifest.assets)) {
    //     const assetPath = this.rt.state.getPathInProject(assetRelativePath);

    //     if (!this.hasAsset(assetPath)) {
    //       delete manifest.assets[assetRelativePath];
    //     }
    //   }

    //   this.refreshEnabledResources();
    // });
  }, 1000);

  *getAssetsConvarDescriptors(): Iterable<ServerVariableDescriptor> {
    const { variables } = this.getManifest();
    if (!variables) {
      return;
    }

    for (const [variableName, variable] of Object.entries(variables)) {
      yield {
        name: variableName,
        value: variable.value,
        setter: variable.setter,
      } as ServerVariableDescriptor;
    }
  }

  private refreshEnabledResources() {
    if (this.worldEditorService.isRunning()) {
      return;
    }

    this.refreshVariables();

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
  }

  private refreshVariables() {
    if (this.worldEditorService.isRunning()) {
      return;
    }

    const { variables } = this.getManifest();
    if (!variables) {
      return;
    }

    this.gameServerService.setVariables([
      ...this.getAssetsConvarDescriptors(),
    ]);
  }
}
