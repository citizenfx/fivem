import { inject, injectable, named } from 'inversify';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution, OpenerService, open } from '@theia/core/lib/browser';
import { PreferenceService } from '@theia/core/lib/browser/preferences';
import { FrontendApplicationStateService } from '@theia/core/lib/browser/frontend-application-state';

import { FileNavigatorWidget } from '@theia/navigator/lib/browser';
import { ContributionProvider } from '@theia/core/lib/common/contribution-provider';

import { FxdkWorkspaceService } from './rebindWorkspaceService';
import URI from '@theia/core/lib/common/uri';
import { CommandService } from '@theia/core';
import { FxdkDataService } from './fxdk-data-service';

@injectable()
export class FxdkProjectContribution implements FrontendApplicationContribution {
  @inject(CommandService)
  protected readonly commandService: CommandService;

  @inject(WorkspacePreferences) protected preferences: WorkspacePreferences;
  @inject(PreferenceService) protected readonly preferenceService: PreferenceService;

  @inject(WorkspaceService)
  protected readonly fxdkWorkspaceService: FxdkWorkspaceService;

  @inject(FileNavigatorWidget)
  protected readonly fileNavigatorWidget: FileNavigatorWidget;

  @inject(FrontendApplicationStateService)
  protected readonly frontendApplicationStateService: FrontendApplicationStateService;

  @inject(ContributionProvider)
  @named(FrontendApplicationContribution)
  protected readonly contributions: ContributionProvider<FrontendApplicationContribution>;

  @inject(OpenerService)
  protected readonly openService: OpenerService;

  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  initialize() {
    console.log('INIT-----------------------------------------------');

    window.parent.postMessage({ type: 'theia:ready' }, '*');

    let shouldReload = false;
    let lastProjectPath;

    window.addEventListener('message', (e) => {
      switch (e.data.type) {
        case 'fxdk:setProject': {
          if (shouldReload && e.data.data.path !== lastProjectPath) {
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
        }
        case 'fxdk:toolbarOpen': {
          const toolbarOpen = e.data.data;

          if (toolbarOpen) {
            document.body.classList.add('fxdk-toolbar-open');
          } else {
            document.body.classList.remove('fxdk-toolbar-open');
          }
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
