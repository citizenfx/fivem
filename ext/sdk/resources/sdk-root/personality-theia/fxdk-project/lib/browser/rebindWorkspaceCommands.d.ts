import { interfaces } from 'inversify';
import URI from '@theia/core/lib/common/uri';
import { SelectionService } from '@theia/core/lib/common/selection-service';
import { Command, CommandContribution, CommandRegistry } from '@theia/core/lib/common/command';
import { MenuContribution, MenuModelRegistry } from '@theia/core/lib/common/menu';
import { FileDialogService } from '@theia/filesystem/lib/browser';
import { OpenerService, OpenHandler, FrontendApplication, LabelProvider } from '@theia/core/lib/browser';
import { UriCommandHandler, UriAwareCommandHandler } from '@theia/core/lib/common/uri-command-handler';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { MessageService } from '@theia/core/lib/common/message-service';
import { WorkspacePreferences } from '@theia/workspace/lib/browser';
import { WorkspaceDeleteHandler } from '@theia/workspace/lib/browser/workspace-delete-handler';
import { WorkspaceDuplicateHandler } from '@theia/workspace/lib/browser/workspace-duplicate-handler';
import { WorkspaceCompareHandler } from '@theia/workspace/lib/browser/workspace-compare-handler';
import { Event } from '@theia/core/lib/common';
import { FileService } from '@theia/filesystem/lib/browser/file-service';
import { FileStat } from '@theia/filesystem/lib/common/files';
export declare namespace WorkspaceCommands {
    const OPEN: Command & {
        dialogLabel: string;
    };
    const OPEN_FILE: Command & {
        dialogLabel: string;
    };
    const OPEN_FOLDER: Command & {
        dialogLabel: string;
    };
    const OPEN_WORKSPACE: Command & {
        dialogLabel: string;
    };
    const OPEN_RECENT_WORKSPACE: Command;
    const CLOSE: Command;
    const NEW_FILE: Command;
    const NEW_FOLDER: Command;
    const FILE_OPEN_WITH: (opener: OpenHandler) => Command;
    const FILE_RENAME: Command;
    const FILE_DELETE: Command;
    const FILE_DUPLICATE: Command;
    const FILE_COMPARE: Command;
    const ADD_FOLDER: Command;
    const REMOVE_FOLDER: Command;
    const SAVE_WORKSPACE_AS: Command;
    const SAVE_AS: Command;
}
export declare class FxdkFileMenuContribution implements MenuContribution {
    registerMenus(registry: MenuModelRegistry): void;
}
export declare class FxdkEditMenuContribution implements MenuContribution {
    registerMenus(registry: MenuModelRegistry): void;
}
export interface DidCreateNewResourceEvent {
    uri: URI;
    parent: URI;
}
export declare class FxdkWorkspaceCommandContribution implements CommandContribution {
    protected readonly labelProvider: LabelProvider;
    protected readonly fileService: FileService;
    protected readonly workspaceService: WorkspaceService;
    protected readonly selectionService: SelectionService;
    protected readonly openerService: OpenerService;
    protected readonly app: FrontendApplication;
    protected readonly messageService: MessageService;
    protected readonly preferences: WorkspacePreferences;
    protected readonly fileDialogService: FileDialogService;
    protected readonly deleteHandler: WorkspaceDeleteHandler;
    protected readonly duplicateHandler: WorkspaceDuplicateHandler;
    protected readonly compareHandler: WorkspaceCompareHandler;
    private readonly onDidCreateNewFileEmitter;
    private readonly onDidCreateNewFolderEmitter;
    get onDidCreateNewFile(): Event<DidCreateNewResourceEvent>;
    get onDidCreateNewFolder(): Event<DidCreateNewResourceEvent>;
    protected fireCreateNewFile(uri: DidCreateNewResourceEvent): void;
    protected fireCreateNewFolder(uri: DidCreateNewResourceEvent): void;
    registerCommands(registry: CommandRegistry): void;
    protected newUriAwareCommandHandler(handler: UriCommandHandler<URI>): UriAwareCommandHandler<URI>;
    protected newMultiUriAwareCommandHandler(handler: UriCommandHandler<URI[]>): UriAwareCommandHandler<URI[]>;
    protected newWorkspaceRootUriAwareCommandHandler(handler: UriCommandHandler<URI>): WorkspaceRootUriAwareCommandHandler;
    /**
     * Returns an error message if the file name is invalid. Otherwise, an empty string.
     *
     * @param name the simple file name of the file to validate.
     * @param parent the parent directory's file stat.
     * @param recursive allow file or folder creation using recursive path
     */
    protected validateFileName(name: string, parent: FileStat, recursive?: boolean): Promise<string>;
    protected trimFileName(name: string): string;
    protected getDirectory(candidate: URI): Promise<FileStat | undefined>;
    protected getParent(candidate: URI): Promise<FileStat | undefined>;
    protected addFolderToWorkspace(uri: URI | undefined): Promise<void>;
    protected areWorkspaceRoots(uris: URI[]): boolean;
    protected isWorkspaceRoot(uri: URI): boolean;
    protected getDefaultFileConfig(): {
        fileName: string;
        fileExtension: string;
    };
    /**
     * Removes the list of folders from the workspace upon confirmation from the user.
     * @param uris the list of folder uris to remove.
     */
    protected removeFolderFromWorkspace(uris: URI[]): Promise<void>;
    protected areMultipleOpenHandlersPresent(openers: OpenHandler[], uri: URI): boolean;
}
export declare class WorkspaceRootUriAwareCommandHandler extends UriAwareCommandHandler<URI> {
    protected readonly workspaceService: WorkspaceService;
    protected readonly selectionService: SelectionService;
    protected readonly handler: UriCommandHandler<URI>;
    constructor(workspaceService: WorkspaceService, selectionService: SelectionService, handler: UriCommandHandler<URI>);
    isEnabled(...args: any[]): boolean;
    isVisible(...args: any[]): boolean;
    protected getUri(...args: any[]): URI | undefined;
}
export declare function rebindWorkspaceCommands(bind: interfaces.Bind, rebind: interfaces.Rebind): void;
//# sourceMappingURL=rebindWorkspaceCommands.d.ts.map