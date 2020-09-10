import { ContainerModule, interfaces, inject, injectable, postConstruct } from 'inversify';
import { WorkspacePreferences } from '@theia/workspace/lib/browser/workspace-preferences';
import { WorkspaceService } from '@theia/workspace/lib/browser';
import { FrontendApplicationContribution } from '@theia/core/lib/browser';
import {
  PreferenceService,
} from '@theia/core/lib/browser/preferences';

import { FileNavigatorWidget } from '@theia/navigator/lib/browser';

import { rebindWorkspacePreferences } from './rebindWorkspacePreferences';
import { rebindWorkspaceService, FxdkWorkspaceService } from './rebindWorkspaceService';
import { rebindWorkspaceCommands } from './rebindWorkspaceCommands';
import { rebindWorkspaceFrontendContribution } from './rebindWorkspaceFrontendContribution';

@injectable()
export class FxdkProjectContribution implements FrontendApplicationContribution {
  @inject(WorkspacePreferences) protected preferences: WorkspacePreferences;
  @inject(PreferenceService) protected readonly preferenceService: PreferenceService;

  @inject(WorkspaceService)
  protected readonly fxdkWorkspaceService: FxdkWorkspaceService;

  @inject(FileNavigatorWidget)
  protected readonly fileNavigatorWidget: FileNavigatorWidget;

  @postConstruct()
  initialize() {

    window.addEventListener('message', (e) => {
      switch (e.data.type) {
        case 'fxdk:addFolder': {
          console.log('adding folder', e.data.data);
          return this.fxdkWorkspaceService.addFolder(e.data.data);
        }
        case 'fxdk:removeFolder': {
          console.log('removing folder', e.data.data);
          return this.fxdkWorkspaceService.removeFolder(e.data.data);
        }
      }
    });
  }
}

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  bind(FxdkProjectContribution).toSelf().inSingletonScope();
  bind(FrontendApplicationContribution).to(FxdkProjectContribution).inSingletonScope();

  rebindWorkspacePreferences(rebind);
  rebindWorkspaceService(bind, rebind);
  rebindWorkspaceCommands(bind, rebind);
  rebindWorkspaceFrontendContribution(bind, rebind);
});
