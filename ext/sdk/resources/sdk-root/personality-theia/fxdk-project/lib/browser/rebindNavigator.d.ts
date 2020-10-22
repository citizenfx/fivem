import { interfaces } from 'inversify';
import { FrontendApplication, FrontendApplicationContribution, KeybindingContribution, KeybindingRegistry, OpenerService, PreferenceService, Widget } from '@theia/core/lib/browser';
import { CommandRegistry, MenuModelRegistry, Mutable, CommandContribution, MenuContribution } from '@theia/core/lib/common';
import { WorkspaceCommandContribution, WorkspacePreferences, WorkspaceService } from '@theia/workspace/lib/browser';
import { TabBarToolbarContribution, TabBarToolbarItem, TabBarToolbarRegistry } from '@theia/core/lib/browser/shell/tab-bar-toolbar';
import { ClipboardService } from '@theia/core/lib/browser/clipboard-service';
import { SelectionService } from '@theia/core/lib/common/selection-service';
import { /*DirNode,*/ FileNode } from '@theia/filesystem/lib/browser';
import { FileNavigatorPreferences } from '@theia/navigator/lib/browser/navigator-preferences';
import { FileNavigatorFilter } from '@theia/navigator/lib/browser/navigator-filter';
import { NavigatorContextKeyService } from '@theia/navigator/lib/browser/navigator-context-key-service';
import { NavigatorDiff } from '@theia/navigator/lib/browser/navigator-diff';
export declare class FxdkFileNavigatorContribution implements FrontendApplicationContribution, TabBarToolbarContribution, CommandContribution, MenuContribution, KeybindingContribution {
    protected readonly fileNavigatorPreferences: FileNavigatorPreferences;
    protected readonly openerService: OpenerService;
    protected readonly fileNavigatorFilter: FileNavigatorFilter;
    protected readonly workspaceService: WorkspaceService;
    protected readonly workspacePreferences: WorkspacePreferences;
    protected readonly clipboardService: ClipboardService;
    protected readonly commandRegistry: CommandRegistry;
    protected readonly tabbarToolbarRegistry: TabBarToolbarRegistry;
    protected readonly contextKeyService: NavigatorContextKeyService;
    protected readonly menuRegistry: MenuModelRegistry;
    protected readonly navigatorDiff: NavigatorDiff;
    protected readonly preferenceService: PreferenceService;
    protected readonly selectionService: SelectionService;
    protected readonly workspaceCommandContribution: WorkspaceCommandContribution;
    constructor(fileNavigatorPreferences: FileNavigatorPreferences, openerService: OpenerService, fileNavigatorFilter: FileNavigatorFilter, workspaceService: WorkspaceService, workspacePreferences: WorkspacePreferences);
    protected init(): Promise<void>;
    private onDidCreateNewResource;
    onStart(app: FrontendApplication): Promise<void>;
    initializeLayout(app: FrontendApplication): Promise<void>;
    registerCommands(registry: CommandRegistry): void;
    protected getSelectedFileNodes(): FileNode[];
    registerMenus(registry: MenuModelRegistry): void;
    registerKeybindings(registry: KeybindingRegistry): void;
    registerToolbarItems(toolbarRegistry: TabBarToolbarRegistry): Promise<void>;
    /**
     * Register commands to the `More Actions...` navigator toolbar item.
     */
    registerMoreToolbarItem: (item: Mutable<TabBarToolbarItem>) => void;
    /**
     * Reveals and selects node in the file navigator to which given widget is related.
     * Does nothing if given widget undefined or doesn't have related resource.
     *
     * @param widget widget file resource of which should be revealed and selected
     */
    selectWidgetFileNode(widget: Widget | undefined): Promise<void>;
    protected onCurrentWidgetChangedHandler(): void;
    /**
     * Collapse file navigator nodes and set focus on first visible node
     * - single root workspace: collapse all nodes except root
     * - multiple root workspace: collapse all nodes, even roots
     */
    collapseFileNavigatorTree(): Promise<void>;
    /**
     * force refresh workspace in navigator
     */
    refreshWorkspace(): Promise<void>;
    private readonly toDisposeAddRemoveFolderActions;
    private updateAddRemoveFolderActions;
}
export declare const rebindNavigator: (bind: interfaces.Bind, rebind: interfaces.Rebind) => void;
//# sourceMappingURL=rebindNavigator.d.ts.map