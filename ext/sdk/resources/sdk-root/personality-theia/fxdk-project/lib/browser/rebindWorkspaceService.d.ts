import { interfaces } from 'inversify';
import { WorkspaceInput } from '@theia/workspace/lib/browser';
import URI from '@theia/core/lib/common/uri';
import { WorkspaceServer } from '@theia/workspace/lib/common';
import { WindowService } from '@theia/core/lib/browser/window/window-service';
import { FrontendApplicationContribution, PreferenceServiceImpl, PreferenceSchemaProvider, LabelProvider } from '@theia/core/lib/browser';
import { EnvVariablesServer } from '@theia/core/lib/common/env-variables';
import { ILogger, Disposable, Emitter, Event, MessageService } from '@theia/core';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import { FileStat, BaseStat } from '@theia/filesystem/lib/common/files';
import { FileService } from '@theia/filesystem/lib/browser/file-service';
import { FileSystemPreferences } from '@theia/filesystem/lib/browser';
export declare class FxdkWorkspaceService implements FrontendApplicationContribution {
    private _folders;
    private _projectName;
    private _projectPath;
    private _projectUri;
    private _workspace;
    private _roots;
    private deferredRoots;
    protected readonly fileService: FileService;
    protected readonly server: WorkspaceServer;
    protected readonly windowService: WindowService;
    protected logger: ILogger;
    protected preferences: WorkspacePreferences;
    protected readonly preferenceImpl: PreferenceServiceImpl;
    protected readonly schemaProvider: PreferenceSchemaProvider;
    protected readonly envVariableServer: EnvVariablesServer;
    protected readonly messageService: MessageService;
    protected readonly labelProvider: LabelProvider;
    protected readonly fsPreferences: FileSystemPreferences;
    protected applicationName: string;
    protected init(): Promise<void>;
    get roots(): Promise<FileStat[]>;
    tryGetRoots(): FileStat[];
    get workspace(): FileStat | undefined;
    protected readonly onWorkspaceChangeEmitter: Emitter<FileStat[]>;
    get onWorkspaceChanged(): Event<FileStat[]>;
    protected readonly onWorkspaceLocationChangedEmitter: Emitter<FileStat | undefined>;
    get onWorkspaceLocationChanged(): Event<FileStat | undefined>;
    setProject({ name, path, folders }: {
        name: any;
        path: any;
        folders: any;
    }): void;
    addFolder(dir: string): void;
    addFolders(dirs: string[]): void;
    setFolders(dirs: string[]): void;
    removeFolder(dir: string): void;
    clearFolders(): void;
    protected updateWorkspace(): Promise<void>;
    protected updateRoots(): Promise<void>;
    protected computeRoots(): Promise<FileStat[]>;
    protected getWorkspaceDataFromFile(): Promise<WorkspaceData | undefined>;
    /**
     * on unload, we set our workspace root as the last recently used on the backend.
     */
    onStop(): void;
    recentWorkspaces(): Promise<string[]>;
    /**
     * Returns `true` if theia has an opened workspace or folder
     * @returns {boolean}
     */
    get opened(): boolean;
    /**
     * Returns `true` if a multiple-root workspace is currently open.
     * @returns {boolean}
     */
    get isMultiRootWorkspaceOpened(): boolean;
    /**
     * Returns `true` if there is an opened workspace, and multi root workspace support is enabled.
     * @returns {boolean}
     */
    get isMultiRootWorkspaceEnabled(): boolean;
    /**
     * Opens directory, or recreates a workspace from the file that `uri` points to.
     */
    open(uri: URI, options?: WorkspaceInput): void;
    /**
     * Clears current workspace root.
     */
    close(): Promise<void>;
    /**
     * returns a FileStat if the argument URI points to an existing directory. Otherwise, `undefined`.
     */
    protected toValidRoot(uri: URI | string | undefined): Promise<FileStat | undefined>;
    /**
     * returns a FileStat if the argument URI points to a file or directory. Otherwise, `undefined`.
     */
    protected toFileStat(uri: URI | string | undefined): Promise<FileStat | undefined>;
    /**
     * Return true if one of the paths in paths array is present in the workspace
     * NOTE: You should always explicitly use `/` as the separator between the path segments.
     */
    containsSome(paths: string[]): Promise<boolean>;
    get saved(): boolean;
    /**
     * Save workspace data into a file
     * @param uri URI or FileStat of the workspace file
     */
    save(uri: URI | FileStat): Promise<void>;
    protected readonly rootWatchers: Map<string, Disposable>;
    protected watchRoots(): Promise<void>;
    protected refreshRootWatchers(): Promise<void>;
    protected watchRoot(root: FileStat): Promise<void>;
    protected getExcludes(uri: string): string[];
    areWorkspaceRoots(uris: URI[]): boolean;
    getWorkspaceRootUri(): void;
}
export interface WorkspaceData {
    folders: Array<{
        path: string;
        name?: string;
    }>;
    settings?: {
        [id: string]: any;
    };
}
export declare namespace WorkspaceData {
    function is(data: any): data is WorkspaceData;
    function buildWorkspaceData(folders: string[] | FileStat[], settings: {
        [id: string]: any;
    } | undefined): WorkspaceData;
    function transformToRelative(data: WorkspaceData, workspaceFile?: FileStat): WorkspaceData;
    function transformToAbsolute(data: WorkspaceData, workspaceFile?: BaseStat): WorkspaceData;
}
export declare function rebindWorkspaceService(bind: interfaces.Bind, rebind: interfaces.Rebind): void;
//# sourceMappingURL=rebindWorkspaceService.d.ts.map