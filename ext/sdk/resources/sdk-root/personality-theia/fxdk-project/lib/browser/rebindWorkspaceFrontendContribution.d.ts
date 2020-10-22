import { interfaces } from 'inversify';
import { CommandContribution, CommandRegistry, MenuContribution, SelectionService } from '@theia/core/lib/common';
import { OpenerService, StorageService, LabelProvider, KeybindingContribution, FrontendApplicationContribution } from '@theia/core/lib/browser';
import { FileDialogService, OpenFileDialogProps } from '@theia/filesystem/lib/browser';
import { ContextKeyService } from '@theia/core/lib/browser/context-key-service';
import { WorkspaceService } from '@theia/workspace/lib/browser/workspace-service';
import { QuickOpenWorkspace } from '@theia/workspace/lib/browser/quick-open-workspace';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import URI from '@theia/core/lib/common/uri';
import { FileService } from '@theia/filesystem/lib/browser/file-service';
import { EncodingRegistry } from '@theia/core/lib/browser/encoding-registry';
import { DisposableCollection } from '@theia/core/lib/common/disposable';
import { PreferenceConfigurations } from '@theia/core/lib/browser/preferences/preference-configurations';
export declare const THEIA_EXT = "theia-workspace";
export declare const VSCODE_EXT = "code-workspace";
export declare enum WorkspaceStates {
    /**
     * The state is `empty` when no workspace is opened.
     */
    empty = "empty",
    /**
     * The state is `workspace` when a workspace is opened.
     */
    workspace = "workspace",
    /**
     * The state is `folder` when a folder is opened. (1 folder)
     */
    folder = "folder"
}
export declare type WorkspaceState = keyof typeof WorkspaceStates;
export declare class FxdkWorkspaceFrontendContribution implements CommandContribution, KeybindingContribution, MenuContribution, FrontendApplicationContribution {
    protected readonly fileService: FileService;
    protected readonly openerService: OpenerService;
    protected readonly workspaceService: WorkspaceService;
    protected readonly workspaceStorage: StorageService;
    protected readonly labelProvider: LabelProvider;
    protected readonly quickOpenWorkspace: QuickOpenWorkspace;
    protected readonly fileDialogService: FileDialogService;
    protected preferences: WorkspacePreferences;
    protected readonly selectionService: SelectionService;
    protected readonly commandRegistry: CommandRegistry;
    protected readonly contextKeyService: ContextKeyService;
    protected readonly encodingRegistry: EncodingRegistry;
    protected readonly preferenceConfigurations: PreferenceConfigurations;
    configure(): void;
    protected readonly toDisposeOnUpdateEncodingOverrides: DisposableCollection;
    protected updateEncodingOverrides(): void;
    protected updateStyles(): void;
    registerCommands(): void;
    registerMenus(): void;
    registerKeybindings(): void;
    /**
     * This is the generic `Open` method. Opens files and directories too. Resolves to the opened URI.
     * Except when you are on either Windows or Linux `AND` running in electron. If so, it opens a file.
     */
    protected doOpen(): Promise<URI | undefined>;
    /**
     * Opens a file after prompting the `Open File` dialog. Resolves to `undefined`, if
     *  - the workspace root is not set,
     *  - the file to open does not exist, or
     *  - it was not a file, but a directory.
     *
     * Otherwise, resolves to the URI of the file.
     */
    protected doOpenFile(): Promise<URI | undefined>;
    /**
     * Opens a folder after prompting the `Open Folder` dialog. Resolves to `undefined`, if
     *  - the workspace root is not set,
     *  - the folder to open does not exist, or
     *  - it was not a directory, but a file resource.
     *
     * Otherwise, resolves to the URI of the folder.
     */
    protected doOpenFolder(): Promise<URI | undefined>;
    /**
     * Opens a workspace after raising the `Open Workspace` dialog. Resolves to the URI of the recently opened workspace,
     * if it was successful. Otherwise, resolves to `undefined`.
     *
     * **Caveat**: this behaves differently on different platforms, the `workspace.supportMultiRootWorkspace` preference value **does** matter,
     * and `electron`/`browser` version has impact too. See [here](https://github.com/eclipse-theia/theia/pull/3202#issuecomment-430884195) for more details.
     *
     * Legend:
     *  - `workspace.supportMultiRootWorkspace` is `false`: => `N`
     *  - `workspace.supportMultiRootWorkspace` is `true`: => `Y`
     *  - Folders only: => `F`
     *  - Workspace files only: => `W`
     *  - Folders and workspace files: => `FW`
     *
     * -----
     *
     * |---------|-----------|-----------|------------|------------|
     * |         | browser Y | browser N | electron Y | electron N |
     * |---------|-----------|-----------|------------|------------|
     * | Linux   |     FW    |     F     |     W      |     F      |
     * | Windows |     FW    |     F     |     W      |     F      |
     * | OS X    |     FW    |     F     |     FW     |     FW     |
     * |---------|-----------|-----------|------------|------------|
     *
     */
    protected doOpenWorkspace(): Promise<URI | undefined>;
    protected openWorkspaceOpenFileDialogProps(): Promise<OpenFileDialogProps>;
    protected closeWorkspace(): Promise<void>;
    protected saveWorkspaceAs(): Promise<void>;
    /**
     * Save source `URI` to target.
     *
     * @param uri the source `URI`.
     */
    protected saveAs(uri: URI): Promise<void>;
    protected updateWorkspaceStateKey(): WorkspaceState;
    private confirmOverwrite;
    private isElectron;
    /**
     * Get the current workspace URI.
     *
     * @returns the current workspace URI.
     */
    private getCurrentWorkspaceUri;
}
export declare function rebindWorkspaceFrontendContribution(bind: interfaces.Bind, rebind: interfaces.Rebind): void;
//# sourceMappingURL=rebindWorkspaceFrontendContribution.d.ts.map