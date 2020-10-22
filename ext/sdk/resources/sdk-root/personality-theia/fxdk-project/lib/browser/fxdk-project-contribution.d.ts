import { FrontendApplicationContribution, OpenerService, WidgetManager } from '@theia/core/lib/browser';
import { FxdkWorkspaceService } from './rebindWorkspaceService';
import { CommandService } from '@theia/core';
import { FxdkDataService } from './fxdk-data-service';
import { FrontendApplicationStateService } from '@theia/core/lib/browser/frontend-application-state';
export declare class FxdkProjectContribution implements FrontendApplicationContribution {
    protected readonly commandService: CommandService;
    protected readonly fxdkWorkspaceService: FxdkWorkspaceService;
    protected readonly openService: OpenerService;
    protected readonly dataService: FxdkDataService;
    protected readonly widgetManager: WidgetManager;
    protected readonly stateService: FrontendApplicationStateService;
    initialize(): void;
    private handleForceReload;
    private handleSetProject;
    private handleSetFolders;
    private handleAddFolders;
    private handleOpenFile;
    private handleData;
    private handleOpenGameView;
    private reachedState;
    private sendMessageToShell;
}
//# sourceMappingURL=fxdk-project-contribution.d.ts.map