import { inject, injectable } from 'inversify';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution, OpenerService, open, WidgetManager } from '@theia/core/lib/browser';

import { FxdkWorkspaceService } from './rebindWorkspaceService';
import URI from '@theia/core/lib/common/uri';
import { CommandService } from '@theia/core';
import { FxdkDataService } from './fxdk-data-service';

import { FxdkGameView, FxdkGameViewContribution } from 'fxdk-game-view/lib/browser/fxdk-game-view-view';

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

  initialize() {
    console.log('INIT-----------------------------------------------');

    window.parent.postMessage({ type: 'theia:ready' }, '*');

    let shouldReload = false;
    let lastProjectPath;

    window.addEventListener('message', (e) => {
      switch (e.data.type) {
        case 'fxdk:forceReload': {
          window.parent.postMessage({ type: 'theia:notReady' }, '*');

          return window.location.reload();
        };
        case 'fxdk:setProject': {
          if (shouldReload && e.data.data.path !== lastProjectPath) {
            console.log('New project set, reloading');
            window.parent.postMessage({ type: 'theia:notReady' }, '*');

            return window.location.reload();
          }

          shouldReload = true;
          lastProjectPath = e.data.data.path;

          console.log('setting or updating project', e.data.data);
          return this.fxdkWorkspaceService.setProject(e.data.data);
        }
        case 'fxdk:addFolders': {
          console.log('adding folders', e.data.data);
          return this.fxdkWorkspaceService.addFolders(e.data.data);
        }
        case 'fxdk:addFolder': {
          console.log('adding folder', e.data.data);
          return this.fxdkWorkspaceService.addFolder(e.data.data);
        }
        case 'fxdk:removeFolder': {
          console.log('removing folder', e.data.data);
          return this.fxdkWorkspaceService.removeFolder(e.data.data);
        }
        case 'fxdk:clearFolders': {
          console.log('clearing folders');
          return this.fxdkWorkspaceService.clearFolders();
        }
        case 'fxdk:openFile': {
          const file: string = e.data.data;

          const lastIndexOfBackslash = file.lastIndexOf('\\');

          const baseUri = new URI('file:///' + file.substr(0, lastIndexOfBackslash).replace(/\\/g, '/'));
          const fileName = file.substr(lastIndexOfBackslash + 1);

          const uri = baseUri.resolve(fileName);

          return open(this.openService, uri);
        }
        case 'fxdk:data': {
          const { data } = e.data;

          if (Array.isArray(data)) {
            data.forEach(({ key, value }) => {
              this.dataService.data[key] = value;
            });
          } else {
            this.dataService.data[data.key] = data.value;
          }

          return;
        }
        case 'fxdk:toolbarOpen': {
          const toolbarOpen = e.data.data;

          if (toolbarOpen) {
            document.body.classList.add('fxdk-toolbar-open');
          } else {
            document.body.classList.remove('fxdk-toolbar-open');
          }

          return;
        }
        case 'fxdk:openGameView': {
          if (!this.widgetManager.tryGetWidget(FxdkGameView.ID)) {
            console.log('Opening Game View');
            this.commandService.executeCommand(FxdkGameViewContribution.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID);
          }

          return;
        }
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
  }
}
