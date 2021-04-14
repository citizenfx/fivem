import { inject, postConstruct, injectable, interfaces } from 'inversify';
import { WorkspaceService, WorkspaceInput } from '@theia/workspace/lib/browser';
import URI from '@theia/core/lib/common/uri';

import { WorkspaceServer } from '@theia/workspace/lib/common';
import { WindowService } from '@theia/core/lib/browser/window/window-service';
import {
  FrontendApplicationContribution, PreferenceServiceImpl, PreferenceSchemaProvider, LabelProvider
} from '@theia/core/lib/browser';
import { Deferred } from '@theia/core/lib/common/promise-util';
import { EnvVariablesServer } from '@theia/core/lib/common/env-variables';
import { ILogger, Disposable, DisposableCollection, Emitter, Event, MessageService } from '@theia/core';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import * as Ajv from 'ajv';
import { FrontendApplicationConfigProvider } from '@theia/core/lib/browser/frontend-application-config-provider';
import { FileStat, BaseStat } from '@theia/filesystem/lib/common/files';
import { FileService } from '@theia/filesystem/lib/browser/file-service';
import { FileSystemPreferences } from '@theia/filesystem/lib/browser';
import { IJSONSchema } from '@theia/core/lib/common/json-schema';
import { WorkspaceSchemaUpdater } from '@theia/workspace/lib/browser/workspace-schema-updater';

@injectable()
export class FxdkWorkspaceService implements FrontendApplicationContribution {

  // FxDK
  private _folders: Set<string> = new Set();
  private _projectName: string = 'none';
  private _projectPath: string = 'none';
  private _projectUri: URI;
  // /FxDK

  private _workspace: FileStat | undefined;

  private _roots: FileStat[] = [];
  private deferredRoots = new Deferred<FileStat[]>();

  @inject(FileService)
  protected readonly fileService: FileService;

  @inject(WorkspaceServer)
  protected readonly server: WorkspaceServer;

  @inject(WindowService)
  protected readonly windowService: WindowService;

  @inject(ILogger)
  protected logger: ILogger;

  @inject(WorkspacePreferences)
  protected preferences: WorkspacePreferences;

  @inject(PreferenceServiceImpl)
  protected readonly preferenceImpl: PreferenceServiceImpl;

  @inject(PreferenceSchemaProvider)
  protected readonly schemaProvider: PreferenceSchemaProvider;

  @inject(EnvVariablesServer)
  protected readonly envVariableServer: EnvVariablesServer;

  @inject(MessageService)
  protected readonly messageService: MessageService;

  @inject(LabelProvider)
  protected readonly labelProvider: LabelProvider;

  @inject(FileSystemPreferences)
  protected readonly fsPreferences: FileSystemPreferences;

  @inject(WorkspaceSchemaUpdater)
  protected readonly schemaUpdater: WorkspaceSchemaUpdater;

  protected applicationName: string;

  @postConstruct()
  protected async init(): Promise<void> {
    this.applicationName = FrontendApplicationConfigProvider.get().applicationName;

    this.fsPreferences.onPreferenceChanged(e => {
      if (e.preferenceName === 'files.watcherExclude') {
        this.refreshRootWatchers();
      }
    });

    // this.updateWorkspace();
  }

  get roots(): Promise<FileStat[]> {
    return this.deferredRoots.promise;
  }
  tryGetRoots(): FileStat[] {
    return this._roots;
  }
  get workspace(): FileStat | undefined {
    if (!this._projectUri) {
      return;
    }

    return {
      isDirectory: false,
      resource: this._projectUri || {
        path: {
          isAbsolute: false,
          isRoot: false,
          name: this._projectName,
        },
        toString: () => {
          return this._projectPath;
        },
      },
    } as any;
  }

  protected readonly onWorkspaceChangeEmitter = new Emitter<FileStat[]>();
  get onWorkspaceChanged(): Event<FileStat[]> {
    return this.onWorkspaceChangeEmitter.event;
  }

  protected readonly onWorkspaceLocationChangedEmitter = new Emitter<FileStat | undefined>();
  get onWorkspaceLocationChanged(): Event<FileStat | undefined> {
    return this.onWorkspaceLocationChangedEmitter.event;
  }

  setProject({ name, path, folders }) {
    this._projectName = name;
    this._projectPath = path;
    this._projectUri = new URI('file:///' + this._projectPath + '/.fxdk/theia-settings.json');
    this._folders = new Set(folders);

    console.log('Opened project in theia', name, path, folders);

    this.onWorkspaceLocationChangedEmitter.fire(this.workspace);
    this.updateWorkspace();
  }

  addFolder(dir: string): void {
    if (!this._folders.has(dir)) {
      this._folders.add(dir);
      this.updateWorkspace();
    }
  }

  addFolders(dirs: string[]): void {
    dirs
      .filter((dir) => !this._folders.has(dir))
      .forEach((dir) => this._folders.add(dir));

    this.updateWorkspace();
  }

  setFolders(dirs: string[]): void {
    this._folders = new Set(dirs);

    this.updateWorkspace();
  }

  removeFolder(dir: string): void {
    this._folders.delete(dir);
    this.updateWorkspace();
  }

  clearFolders(): void {
    this._folders = new Set();
    this.updateWorkspace();
  }

  protected async updateWorkspace(): Promise<void> {
    await this.updateRoots();
    this.watchRoots();
  }

  protected async updateRoots(): Promise<void> {
    const newRoots = await this.computeRoots();
    let rootsChanged = false;
    if (newRoots.length !== this._roots.length || newRoots.length === 0) {
      rootsChanged = true;
    } else {
      for (const newRoot of newRoots) {
        if (!this._roots.some(r => r.resource.toString() === newRoot.resource.toString())) {
          rootsChanged = true;
          break;
        }
      }
    }
    if (rootsChanged) {
      this._roots = newRoots;
      this.deferredRoots.resolve(this._roots); // in order to resolve first
      this.deferredRoots = new Deferred<FileStat[]>();
      this.deferredRoots.resolve(this._roots);

      this.onWorkspaceChangeEmitter.fire(this._roots);
    }
  }

  protected async computeRoots(): Promise<FileStat[]> {
    const roots: FileStat[] = [];

    for (const path of this._folders) {
      const valid = await this.toValidRoot(path);
      if (valid) {
        roots.push(valid);
      } else {
        roots.push(FileStat.dir(path));
      }
    }

    return roots;
  }

  protected async getWorkspaceDataFromFile(): Promise<WorkspaceData | undefined> {
    return {
      folders: [...this._folders.values()].map((path) => ({ path })),
    };
  }

  /**
   * on unload, we set our workspace root as the last recently used on the backend.
   */
  onStop(): void {
    return;
  }

  async recentWorkspaces(): Promise<string[]> {
    return [];
  }

  /**
   * Returns `true` if theia has an opened workspace or folder
   * @returns {boolean}
   */
  get opened(): boolean {
    return true;
  }

  /**
   * Returns `true` if a multiple-root workspace is currently open.
   * @returns {boolean}
   */
  get isMultiRootWorkspaceOpened(): boolean {
    return !!this.workspace && true;
  }

  /**
   * Returns `true` if there is an opened workspace, and multi root workspace support is enabled.
   * @returns {boolean}
   */
  get isMultiRootWorkspaceEnabled(): boolean {
    return this.opened && true;
  }

  /**
   * Opens directory, or recreates a workspace from the file that `uri` points to.
   */
  open(uri: URI, options?: WorkspaceInput): void {
    return;
  }

  /**
   * Clears current workspace root.
   */
  async close(): Promise<void> {
    this._roots.length = 0;
  }

  /**
   * returns a FileStat if the argument URI points to an existing directory. Otherwise, `undefined`.
   */
  protected async toValidRoot(uri: URI | string | undefined): Promise<FileStat | undefined> {
    const fileStat = await this.toFileStat(uri);
    if (fileStat && fileStat.isDirectory) {
      return fileStat;
    }
    return undefined;
  }

  /**
   * returns a FileStat if the argument URI points to a file or directory. Otherwise, `undefined`.
   */
  protected async toFileStat(uri: URI | string | undefined): Promise<FileStat | undefined> {
    if (!uri) {
      return undefined;
    }
    let uriStr = uri.toString();
    try {
      if (uriStr.endsWith('/')) {
        uriStr = uriStr.slice(0, -1);
      }
      const normalizedUri = new URI(uriStr).normalizePath();
      return await this.fileService.resolve(normalizedUri);
    } catch (error) {
      return undefined;
    }
  }

  /**
   * Return true if one of the paths in paths array is present in the workspace
   * NOTE: You should always explicitly use `/` as the separator between the path segments.
   */
  async containsSome(paths: string[]): Promise<boolean> {
    await this.roots;
    if (this.opened) {
      for (const root of this._roots) {
        const uri = root.resource;
        for (const path of paths) {
          const fileUri = uri.resolve(path);
          const exists = await this.fileService.exists(fileUri);
          if (exists) {
            return exists;
          }
        }
      }
    }
    return false;
  }

  get saved(): boolean {
    return !!this._workspace && !this._workspace.isDirectory;
  }

  /**
   * Save workspace data into a file
   * @param uri URI or FileStat of the workspace file
   */
  async save(uri: URI | FileStat): Promise<void> {
    return;
  }

  protected readonly rootWatchers = new Map<string, Disposable>();

  protected async watchRoots(): Promise<void> {
    const rootUris = new Set(this._roots.map(r => r.resource.toString()));
    for (const [uri, watcher] of this.rootWatchers.entries()) {
      if (!rootUris.has(uri)) {
        watcher.dispose();
      }
    }
    for (const root of this._roots) {
      this.watchRoot(root);
    }
  }

  protected async refreshRootWatchers(): Promise<void> {
    for (const watcher of this.rootWatchers.values()) {
      watcher.dispose();
    }
    await this.watchRoots();
  }

  protected async watchRoot(root: FileStat): Promise<void> {
    const uriStr = root.resource.toString();
    if (this.rootWatchers.has(uriStr)) {
      return;
    }
    const excludes = this.getExcludes(uriStr);
    const watcher = this.fileService.watch(new URI(uriStr), {
      recursive: true,
      excludes
    });
    this.rootWatchers.set(uriStr, new DisposableCollection(
      watcher,
      Disposable.create(() => this.rootWatchers.delete(uriStr))
    ));
  }

  protected getExcludes(uri: string): string[] {
    const patterns = this.fsPreferences.get('files.watcherExclude', undefined, uri);
    return Object.keys(patterns).filter(pattern => patterns[pattern]);
  }

  // Filler
  getWorkspaceRootUri() {
    return;
  }

  areWorkspaceRoots(uris: URI[]): boolean {
    if (!uris.length) {
      return false;
    }
    const rootUris = new Set(this.tryGetRoots().map(root => root.resource.toString()));
    return uris.every(uri => rootUris.has(uri.toString()));
  }

  async updateSchema(key: string, schema?: IJSONSchema): Promise<boolean> {
    return this.schemaUpdater.updateSchema({ key, schema });
  }
}

export interface WorkspaceData {
  folders: Array<{ path: string, name?: string }>;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  settings?: { [id: string]: any };
}

export namespace WorkspaceData {
  const validateSchema = new Ajv().compile({
    type: 'object',
    properties: {
      folders: {
        description: 'Root folders in the workspace',
        type: 'array',
        items: {
          type: 'object',
          properties: {
            path: {
              type: 'string',
            }
          },
          required: ['path']
        }
      },
      settings: {
        description: 'Workspace preferences',
        type: 'object'
      }
    },
    required: ['folders']
  });

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  export function is(data: any): data is WorkspaceData {
    return !!validateSchema(data);
  }

  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  export function buildWorkspaceData(folders: string[] | FileStat[], settings: { [id: string]: any } | undefined): WorkspaceData {
    let roots: string[] = [];
    if (folders.length > 0) {
      if (typeof folders[0] !== 'string') {
        roots = (<FileStat[]>folders).map(folder => folder.resource.toString());
      } else {
        roots = <string[]>folders;
      }
    }
    const data: WorkspaceData = {
      folders: roots.map(folder => ({ path: folder }))
    };
    if (settings) {
      data.settings = settings;
    }
    return data;
  }

  export function transformToRelative(data: WorkspaceData, workspaceFile?: FileStat): WorkspaceData {
    const folderUris: string[] = [];
    const workspaceFileUri = new URI(workspaceFile ? workspaceFile.resource.toString() : '').withScheme('file');
    for (const { path } of data.folders) {
      const folderUri = new URI(path).withScheme('file');
      const rel = workspaceFileUri.parent.relative(folderUri);
      if (rel) {
        folderUris.push(rel.toString());
      } else {
        folderUris.push(folderUri.toString());
      }
    }
    return buildWorkspaceData(folderUris, data.settings);
  }

  export function transformToAbsolute(data: WorkspaceData, workspaceFile?: BaseStat): WorkspaceData {
    if (workspaceFile) {
      const folders: string[] = [];
      for (const folder of data.folders) {
        const path = folder.path;
        if (path.startsWith('file:///')) {
          folders.push(path);
        } else {
          folders.push(workspaceFile.resource.withScheme('file').parent.resolve(path).toString());
        }

      }
      return Object.assign(data, buildWorkspaceData(folders, data.settings));
    }
    return data;
  }
}


export function rebindWorkspaceService(bind: interfaces.Bind, rebind: interfaces.Rebind) {
  bind(FxdkWorkspaceService).toSelf().inSingletonScope();

  rebind(WorkspaceService).toService(FxdkWorkspaceService as any);
}
