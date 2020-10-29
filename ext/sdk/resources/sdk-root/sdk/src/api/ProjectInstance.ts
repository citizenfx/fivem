import * as fs from 'fs';
import * as path from 'path';
import * as mkdirp from 'mkdirp';
import * as chokidar from 'chokidar';
import * as rimrafSync from 'rimraf';
import { promisify } from 'util';
import {
  ApiClient,
  AssetDeleteRequest,
  AssetMeta,
  AssetRenameRequest,
  MoveEntryRequest,
  Project,
  ProjectFsTree,
  ProjectManifest,
  ProjectManifestResource,
  ProjectPathsState,
  ProjectResources,
  RelinkResourcesRequest,
  ServerUpdateChannel,
  serverUpdateChannels,
} from "./api.types";
import { projectApi } from './events';
import { createLock, debounce, getEnabledResourcesPaths, getResourceConfig, getProjectResources } from './utils';
import { fxdkAssetFilename, fxdkProjectFilename } from './constants';
import { EntryMetaExtras, ExplorerApi } from './ExplorerApi';
import { SystemEvent, systemEvents } from './api.events';


const rimraf = promisify(rimrafSync);

enum FsTreeUpdateType {
  add,
  addDir,
  change,
  unlink,
  unlinkDir,
}

export interface ProjectCreateRequest {
  path: string,
  name: string,
}


interface Silentable {
  silent?: boolean,
}

export interface SetManifestOptions extends Silentable {
}

export interface AssetMetaAccessorOptions extends Silentable {
  forceShadow?: boolean,
  forceReal?: boolean,
}

export interface GetAssetMetaOptions extends AssetMetaAccessorOptions {
}

export interface SetAssetMetaOptions extends AssetMetaAccessorOptions {
}

export class ProjectInstance {
  static async openProject(projectPath: string, client: ApiClient, explorer: ExplorerApi): Promise<ProjectInstance> {
    const project = new ProjectInstance(projectPath, client, explorer);

    await project.init();

    return project;
  }

  static async createProject(request: ProjectCreateRequest, client: ApiClient, explorer: ExplorerApi): Promise<ProjectInstance> {
    client.log('Creating project', request);

    const projectPath = path.join(request.path, request.name);
    const projectShadowRootPath = path.join(projectPath, '.fxdk/shadowRoot');

    const projectManifestPath = path.join(projectPath, fxdkProjectFilename);
    const projectManifest: ProjectManifest = {
      name: request.name,
      serverUpdateChannel: serverUpdateChannels.recommended,
      createdAt: new Date().toISOString(),
      resources: {},
      pathsState: {},
    };

    const theiaSettingsPath = path.join(projectPath, '.fxdk/theia-settings.json');
    const theiaSettings = {
      folders: [],
      settings: {},
    };

    // It will create projectPath as well
    await mkdirp(projectShadowRootPath)

    await Promise.all([
      // Write project manifest
      fs.promises.writeFile(projectManifestPath, JSON.stringify(projectManifest, null, 2)),

      // Write theia-personality settings
      fs.promises.writeFile(theiaSettingsPath, JSON.stringify(theiaSettings, null, 2)),
    ]);

    const projectInstance = new ProjectInstance(projectPath, client, explorer);
    await projectInstance.init();

    return projectInstance;
  }

  private fsTree: ProjectFsTree;
  private manifest: ProjectManifest;
  private resources: ProjectResources;

  private watcher: chokidar.FSWatcher;

  private reconcileLock = createLock();

  private disposers: Function[] = [];

  get project(): Project {
    return {
      path: this.path,
      fsTree: this.fsTree,
      manifest: this.manifest,
    };
  }

  get manifestPath(): string {
    return path.join(this.path, fxdkProjectFilename);
  }

  get storagePath(): string {
    return path.join(this.path, '.fxdk');
  }

  get shadowPath(): string {
    return path.join(this.storagePath, 'shadowRoot');
  }

  get enabledResourcesPaths(): string[] {
    return getEnabledResourcesPaths(this.project, this.resources);
  }

  get entryMetaExtras(): EntryMetaExtras {
    return {
      assetMeta: async (entryPath: string) => {
        return this.getAssetMeta(entryPath, { silent: true });
      },
    };
  }

  constructor(
    private path: string,
    private readonly client: ApiClient,
    private readonly explorerApi: ExplorerApi,
  ) {
    this.log('opening');

    this.disposers.push(
      this.client.on(projectApi.setPathsState, (pathsState: ProjectPathsState) => this.setPathsState(pathsState)),
      this.client.on(projectApi.setServerUpdateChannel, (updateChannel: ServerUpdateChannel) => this.setServerUpdateChannel(updateChannel)),

      this.client.on(projectApi.setResourceEnabled, ({ resourceName, enabled }) => this.setResourceEnabled(resourceName, enabled)),
      this.client.on(projectApi.setResourceConfig, ({ resourceName, config }) => this.setResourceConfig(resourceName, config)),
      this.client.on(projectApi.updateResources, () => this.readAndNotifyFsTree()),

      this.client.on(projectApi.createDirectory, ({ directoryPath, directoryName }) => this.createDirectory(directoryPath, directoryName)),
      this.client.on(projectApi.deleteDirectory, ({ directoryPath }) => this.deleteDirectory(directoryPath)),
      this.client.on(projectApi.renameDirectory, ({ directoryPath, newDirectoryName }) => this.renameDirectory(directoryPath, newDirectoryName)),

      this.client.on(projectApi.createFile, ({ filePath, fileName }) => this.createFile(filePath, fileName)),
      this.client.on(projectApi.deleteFile, ({ filePath }) => this.deleteFile(filePath)),
      this.client.on(projectApi.renameFile, ({ filePath, newFileName }) => this.renameFile(filePath, newFileName)),

      this.client.on(projectApi.moveEntry, (request: MoveEntryRequest) => this.moveEntry(request)),
    );
  }

  log(msg: string, ...args) {
    this.client.log(`[ProjectInstance: ${this.path}]`, msg, ...args);
  }

  async init() {
    this.log('initializing...');

    await Promise.all([
      this.readManifest(),
      this.readFsTree(),
    ]);

    this.watchProject();

    this.log('initialized');
  }

  async close() {
    this.log('closing...');
    systemEvents.emit(SystemEvent.forceStopServer);

    this.disposers.forEach((disposer) => disposer());

    if (this.watcher) {
      await this.watcher.close();
    }

    this.log('closed');
  }

  getProjectResources(): ProjectResources {
    return this.resources;
  }

  private setManifestDebounced = debounce(async (options?: SetManifestOptions) => {
    await this.writeManifest();

    if (!options?.silent) {
      this.notifyProjectUpdated();
    }
  }, 10);

  async setManifest(manifest: ProjectManifest, options?: SetManifestOptions) {
    this.manifest = manifest;

    this.setManifestDebounced(options);
  }

  async setResourceEnabled(resourceName: string, enabled: boolean) {
    this.manifest.resources[resourceName] = {
      ...getResourceConfig(this.manifest, resourceName),
      enabled,
    };

    this.setManifest(this.manifest);
  }

  async setResourcesEnabled(resourceNames: string[], enabled: boolean) {
    resourceNames.forEach((resourceName) => {
      this.setResourceEnabled(resourceName, enabled);
    });
  }

  async setResourceConfig(resourceName: string, config: Partial<ProjectManifestResource>) {
    this.manifest.resources[resourceName] = {
      ...getResourceConfig(this.manifest, resourceName),
      ...config,
    };

    this.writeManifest();
    this.notifyProjectUpdated();
  }

  setPathsState(pathsState: ProjectPathsState) {
    this.log('saving paths state', pathsState);

    this.manifest.pathsState = pathsState;

    this.setManifestDebounced();
  }

  /**
   * Returns path but in shadow root
   *
   * @param requestPath
   */
  getPathInShadow(requestPath: string): string {
    let relativePath = requestPath;

    if (path.isAbsolute(requestPath)) {
      relativePath = path.relative(this.path, requestPath);
    }

    return path.join(this.shadowPath, relativePath);
  }

  setServerUpdateChannel(updateChannel: ServerUpdateChannel) {
    if (this.manifest.serverUpdateChannel !== updateChannel) {
      systemEvents.emit(SystemEvent.forceStopServer);

      this.manifest.serverUpdateChannel = updateChannel;

      this.setManifestDebounced();
    }
  }

  /**
   * Returns meta filepath that exist in assetPath
   * Otherwise will return meta filepath in shadow root
   *
   * Can be overridden by options
   *
   * @param assetPath
   */
  async getAssetMetaPath(assetPath: string, options?: AssetMetaAccessorOptions): Promise<string> {
    const shadowPath = path.join(this.getPathInShadow(assetPath), fxdkAssetFilename);
    const realPath = path.join(assetPath, fxdkAssetFilename);

    if (options?.forceShadow) {
      return shadowPath;
    }

    if (options?.forceReal) {
      return realPath;
    }

    try {
      await fs.promises.stat(realPath);

      return realPath;
    } catch (e) { }

    return shadowPath;
  }

  async getAssetMeta(assetPath: string, options?: GetAssetMetaOptions): Promise<AssetMeta | null> {
    const assetMetaFilepath = await this.getAssetMetaPath(assetPath, options);

    try {
      await fs.promises.stat(assetMetaFilepath);

      const assetMeta = await fs.promises.readFile(assetMetaFilepath);

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
    await mkdirp(path.dirname(assetMetaFilepath));

    await fs.promises.writeFile(assetMetaFilepath, JSON.stringify(assetMeta, null, 2));
  }

  // Directories methods
  async createDirectory(directoryPath: string, name: string) {
    const directoryFullPath = path.join(directoryPath, name);

    await mkdirp(directoryFullPath);
  }

  async deleteDirectory(directoryPath: string) {
    await rimraf(directoryPath);
  }

  async renameDirectory(directoryPath: string, newName: string) {
    const newDirectoryPath = path.join(path.dirname(directoryPath), newName);

    await fs.promises.rename(directoryPath, newDirectoryPath);
  }
  // /Directory methods

  // Files methods
  async createFile(filePath: string, name: string) {
    const fileFullPath = path.join(filePath, name);

    await fs.promises.writeFile(fileFullPath, '');
  }

  async deleteFile(filePath: string) {
    await fs.promises.unlink(filePath);
  }

  async renameFile(filePath: string, newName: string) {
    const newFilePath = path.join(path.dirname(filePath), newName);

    await fs.promises.rename(filePath, newFilePath);
  }
  // /Files methods

  // FS methods
  async moveEntry(request: MoveEntryRequest) {
    this.reconcileLock.withLock(async () => {
      const { sourcePath, targetPath } = request;

      const newPath = path.join(targetPath, path.basename(sourcePath));

      await fs.promises.rename(sourcePath, newPath);

      // const [source, target] = await Promise.all([
      //   this.explorerApi.getEntry(sourcePath, this.entryMetaExtras),
      //   this.explorerApi.getEntry(targetPath, this.entryMetaExtras),
      // ]);
    });
  }
  // /FS methods

  readAndNotifyFsTree = debounce(async () => {
    this.log('reading fs tree, reconciling resources and sending update');

    await this.readFsTree();
    await this.reconcileResourcesInManifest();

    this.notifyProjectUpdated();
  }, 50);

  /**
   * Extra handler for asset rename
   *
   * @param request
   */
  async handleAssetRename(request: AssetRenameRequest) {
    this.reconcileLock.withLock(async () => {
      const { newAssetName } = request;

      const oldAssetName = path.basename(request.assetPath);

      const resourceMeta = this.manifest.resources[oldAssetName];
      if (resourceMeta) {
        this.manifest.resources[newAssetName] = {
          ...resourceMeta,
          name: newAssetName,
        };

        await this.writeManifest();
      }
    });
  }

  /**
   * Extra handler for asset delete
   *
   * @param request
   */
  async handleAssetDelete(request: AssetDeleteRequest) {
    this.reconcileLock.withLock(async () => {
      const { assetPath } = request;

      const assetName = path.basename(assetPath);

      const resourceMeta = this.manifest.resources[assetName];
      if (resourceMeta) {
        delete this.manifest.resources[assetName];

        await this.writeManifest();
      }
    });
  }

  /**
   * Send partial project update
   */
  private notifyProjectUpdated() {
    this.client.emit(projectApi.update, this.project);
  }

  private async readFsTree(): Promise<ProjectFsTree> {
    const pathsMap = await this.explorerApi.readDirRecursively(this.path, this.entryMetaExtras);
    const entries = pathsMap[this.path];

    this.fsTree = {
      entries,
      pathsMap,
    };
    this.resources = getProjectResources(this.project);

    return this.fsTree;
  }

  private async readManifest(): Promise<ProjectManifest> {
    const manifestContent = await fs.promises.readFile(this.manifestPath);

    return this.manifest = {
      pathsState: {},
      resources: {},
      serverUpdateChannel: serverUpdateChannels.recommended,
      ...JSON.parse(manifestContent.toString('utf8')),
    };
  }

  private async writeManifest() {
    await fs.promises.writeFile(this.manifestPath, JSON.stringify({
      ...this.manifest,
      updatedAt: new Date().toISOString(),
    }, null, 2));
  }

  private async watchProject() {
    this.client.log('Start watching project', this.path);

    this.watcher = chokidar.watch(this.path, {
      ignored: /(^|[\/\\])\../, // ignore dotfiles
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

  private async handleFsTreeUpdate(updateType: FsTreeUpdateType, updatedPath: string) {
    if (updateType !== FsTreeUpdateType.change) {
      this.readAndNotifyFsTree();
    }

    switch (updateType) {
      case FsTreeUpdateType.change:
      case FsTreeUpdateType.add:
        const enabledResourcesPaths = this.enabledResourcesPaths;
        const updatedResourcePath = enabledResourcesPaths.find((enabledResourcePath) => updatedPath.startsWith(enabledResourcePath));

        if (updatedResourcePath) {
          const resourceName = path.basename(updatedResourcePath);
          const resourceConfig = getResourceConfig(this.manifest, resourceName);

          if (resourceConfig.restartOnChange) {
            systemEvents.emit(SystemEvent.restartResource, resourceName);
          }
        }

        break;
    }
  }

  private async reconcileResourcesInManifest() {
    await this.reconcileLock.waitForUnlock();

    const manifestResources = {};

    Object.values(this.resources)
      .forEach(({ name }) => {
        manifestResources[name] = this.manifest.resources[name];
      });

    if (Object.keys(this.manifest.resources).length !== Object.keys(manifestResources).length) {
      this.manifest.resources = manifestResources;
      await this.writeManifest();

      this.emitResourcesRelinkRequest();
    }
  }

  private emitResourcesRelinkRequest() {
    const relinkRequest: RelinkResourcesRequest = {
      projectPath: this.path,
      enabledResourcesPaths: this.enabledResourcesPaths,
    };

    systemEvents.emit(SystemEvent.relinkResources, relinkRequest);
  }
}
