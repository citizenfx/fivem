import { inject, injectable } from 'inversify';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution, OpenerService, open, WidgetManager } from '@theia/core/lib/browser';

import { FxdkWorkspaceService } from './rebindWorkspaceService';
import URI from '@theia/core/lib/common/uri';
import { CommandService } from '@theia/core';
import { FxdkDataService, StructuredMessage } from './fxdk-data-service';

import { FxdkGameView, FxdkGameViewContribution } from 'fxdk-game-view/lib/browser/fxdk-game-view-view';
import { FrontendApplicationState, FrontendApplicationStateService } from '@theia/core/lib/browser/frontend-application-state';

const stateToNumber: Record<FrontendApplicationState, number> = {
  init: 0,
  started_contributions: 1,
  attached_shell: 2,
  initialized_layout: 3,
  ready: 4,
  closing_window: 5,
};

function mapStateToNumber(state: FrontendApplicationState): number {
  return stateToNumber[state];
}

function mapFxdkFolderToTheiaFolder(folder: string): string {
  return folder.replace(/\\/g, '/');
}

@injectable()
export class FxdkProjectContribution implements FrontendApplicationContribution {
  @inject(CommandService)
  protected readonly commandService: CommandService;

  @inject(WorkspaceService)
  protected readonly fxdkWorkspaceService: FxdkWorkspaceService;

  @inject(OpenerService)
  protected readonly openService: OpenerService;

  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  @inject(WidgetManager)
  protected readonly widgetManager: WidgetManager;

  @inject(FrontendApplicationStateService)
  protected readonly stateService: FrontendApplicationStateService;

  initialize() {
    window.parent.postMessage({ type: 'theia:ready' }, '*');

    window.addEventListener('message', (e) => {
      if (typeof e.data !== 'object' || e.data === null) {
        return;
      }

      const { type, data } = e.data;

      if (type.indexOf('fxdk:') !== 0) {
        return;
      }

      const methodBase = type.substr(5);
      const method = 'handle' + methodBase[0].toUpperCase() + methodBase.substr(1);

      if (typeof this[method] !== 'undefined') {
        return this[method](data);
      }
    });

    document.addEventListener('contextmenu', (e) => e.preventDefault());
  }

  private handleForceReload() {
    this.sendMessageToShell('theia:notReady');

    return window.location.reload();
  }

  private handleSetProject(project: { name: string, path: string, folders: string[] }) {
    console.log('setting or updating project', project);

    const theiaProject = {
      name: project.name,
      path: project.path,
      folders: project.folders.map(mapFxdkFolderToTheiaFolder),
    }

    return this.fxdkWorkspaceService.setProject(theiaProject);
  }

  private handleSetFolders(folders: string[]) {
    console.log('setting folders', folders);

    return this.fxdkWorkspaceService.setFolders(folders.map(mapFxdkFolderToTheiaFolder));
  }

  private handleAddFolders(folders: string[]) {
    console.log('adding folders', folders);
    return this.fxdkWorkspaceService.addFolders(folders.map(mapFxdkFolderToTheiaFolder));
  }

  private handleOpenFile(file: string) {
    if (!this.reachedState('ready')) {
      return;
    }

    const lastIndexOfBackslash = file.lastIndexOf('\\');

    const baseUri = new URI('file:///' + file.substr(0, lastIndexOfBackslash).replace(/\\/g, '/'));
    const fileName = file.substr(lastIndexOfBackslash + 1);

    const uri = baseUri.resolve(fileName);

    return open(this.openService, uri);
  }

  private handleData(data: { key: string, value: any } | Array<{ key: string, value: any }>) {
    if (!this.reachedState('ready')) {
      return;
    }

    if (Array.isArray(data)) {
      data.forEach(({ key, value }) => {
        this.dataService.data[key] = value;
      });
    } else {
      this.dataService.data[data.key] = data.value;
    }
  }

  private handleOpenGameView() {
    if (!this.reachedState('ready')) {
      return;
    }

    if (!this.widgetManager.tryGetWidget(FxdkGameView.ID)) {
      console.log('Opening Game View');
      this.commandService.executeCommand(FxdkGameViewContribution.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID);
    }
  }

  private handleServerOutput(output: string) {
    this.dataService.setBufferedServerOutput(output);
  }

  private handleServerOutputStructured(msg: StructuredMessage) {
    this.dataService.receiveStructuredServerMessage(msg);
  }

  private handleClearServerOutput() {
    this.dataService.clearAllServerOutputs();
  }

  private handleGameStructuredMessage(msg: StructuredMessage) {
    this.dataService.receiveStructuredGameMessage(msg);
  }

  private handleClearGameOutput() {
    this.dataService.clearGameOutput();
  }

  private reachedState(state: FrontendApplicationState) {
    const stateNumber = mapStateToNumber(state);
    const currentStateNumber = mapStateToNumber(this.stateService.state);

    return currentStateNumber >= stateNumber;
  }

  private sendMessageToShell(type: string, data?: any) {
    window.parent.postMessage({ type, data }, '*');
  }
}
