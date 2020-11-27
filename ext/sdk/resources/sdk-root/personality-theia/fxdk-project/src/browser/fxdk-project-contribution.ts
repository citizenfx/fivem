import { inject, injectable } from 'inversify';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution, OpenerService, open, WidgetManager } from '@theia/core/lib/browser';

import { FxdkWorkspaceService } from './rebindWorkspaceService';
import URI from '@theia/core/lib/common/uri';
import { CommandService } from '@theia/core';
import { FxdkDataService } from './fxdk-data-service';

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

    // Setup our styles for theia:icon
    const styleNode = document.createElement('style');
    styleNode.innerHTML = `
    .fxdk-toolbar-open .theia-icon {
      display: none !important;
    }
    `;

    document.head.appendChild(styleNode);

    document.addEventListener('contextmenu', (e) => e.preventDefault());
  }

  private handleForceReload() {
    this.sendMessageToShell('theia:notReady');

    return window.location.reload();
  }

  private handleSetProject(project: { name: string, path: string, folders: string[] }) {
    console.log('setting or updating project', project);
    return this.fxdkWorkspaceService.setProject(project);
  }

  private handleAddFolders(folders: string[]) {
    console.log('adding folders', folders);
    return this.fxdkWorkspaceService.addFolders(folders);
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

  private handleToolbarOpen(toolbarOpen: boolean) {
    if (toolbarOpen) {
      document.body.classList.add('fxdk-toolbar-open');
    } else {
      document.body.classList.remove('fxdk-toolbar-open');
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

  private reachedState(state: FrontendApplicationState) {
    const stateNumber = mapStateToNumber(state);
    const currentStateNumber = mapStateToNumber(this.stateService.state);

    return currentStateNumber >= stateNumber;
  }

  private sendMessageToShell(type: string, data?: any) {
    window.parent.postMessage({ type, data }, '*');
  }
}
