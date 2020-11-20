import { ContainerModule, interfaces } from 'inversify';
import { FrontendApplicationContribution } from '@theia/core/lib/browser';

import { rebindWorkspacePreferences } from './rebindWorkspacePreferences';
import { rebindWorkspaceService } from './rebindWorkspaceService';
import { rebindWorkspaceCommands } from './rebindWorkspaceCommands';
import { rebindWorkspaceFrontendContribution } from './rebindWorkspaceFrontendContribution';
import { rebindApplicationShell } from './rebindApplicationShell';
import { rebindNavigator } from './rebindNavigator';
import { FxdkProjectContribution } from './fxdk-project-contribution';
import { FxdkDataService } from './fxdk-data-service';
import { FxdkMenuContribution } from './fxdk-menu-contribution';
import { CommandContribution, MenuContribution } from '@theia/core';


export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  bind(FxdkDataService).toSelf().inSingletonScope();

  bind(FxdkMenuContribution).toSelf().inSingletonScope();
  bind(MenuContribution).to(FxdkMenuContribution);
  bind(CommandContribution).to(FxdkMenuContribution);

  bind(FxdkProjectContribution).toSelf().inSingletonScope();
  bind(FrontendApplicationContribution).to(FxdkProjectContribution).inSingletonScope();

  rebindApplicationShell(bind, rebind);
  rebindNavigator(bind, rebind);

  rebindWorkspacePreferences(rebind);
  rebindWorkspaceService(bind, rebind);
  rebindWorkspaceCommands(bind, rebind);
  rebindWorkspaceFrontendContribution(bind, rebind);
});
