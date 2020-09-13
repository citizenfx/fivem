import { ContainerModule, interfaces, inject, injectable, named } from 'inversify';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution, OpenerService, open } from '@theia/core/lib/browser';
import { PreferenceService } from '@theia/core/lib/browser/preferences';
import { FrontendApplicationStateService } from '@theia/core/lib/browser/frontend-application-state';

import { FileNavigatorWidget } from '@theia/navigator/lib/browser';
import { ContributionProvider } from '@theia/core/lib/common/contribution-provider';

import { rebindWorkspacePreferences } from './rebindWorkspacePreferences';
import { rebindWorkspaceService, FxdkWorkspaceService } from './rebindWorkspaceService';
import { rebindWorkspaceCommands } from './rebindWorkspaceCommands';
import { rebindWorkspaceFrontendContribution } from './rebindWorkspaceFrontendContribution';
import { rebindApplicationShell } from './rebindApplicationShell';
import URI from '@theia/core/lib/common/uri';
import { rebindNavigator } from './rebindNavigator';

@injectable()
export class FxdkProjectContribution implements FrontendApplicationContribution {
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

          const baseUri = new URI('file:///' + file.substr(0, lastIndexOfBackslash));
          const fileName = file.substr(lastIndexOfBackslash + 1);

          const uri = baseUri.resolve(fileName);

          return open(this.openService, uri);
        }
      }
    });
  }
}

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  bind(FxdkProjectContribution).toSelf().inSingletonScope();
  bind(FrontendApplicationContribution).to(FxdkProjectContribution).inSingletonScope();

  rebindApplicationShell(bind, rebind);
  rebindNavigator(bind, rebind);

  rebindWorkspacePreferences(rebind);
  rebindWorkspaceService(bind, rebind);
  rebindWorkspaceCommands(bind, rebind);
  rebindWorkspaceFrontendContribution(bind, rebind);
});
