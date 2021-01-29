import '../../src/browser/styles/console.css';

import { ContainerModule, interfaces } from 'inversify';
import { bindViewContribution, FrontendApplicationContribution, WidgetFactory } from '@theia/core/lib/browser';

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
import { ServerConsole, ServerConsoleViewContribution, SERVER_CONSOLE_WIDGET_ID } from './console/server-console';


export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  bind(FxdkDataService).toSelf().inSingletonScope();

  bind(FxdkMenuContribution).toSelf().inSingletonScope();
  bind(MenuContribution).toService(FxdkMenuContribution);
  bind(CommandContribution).toService(FxdkMenuContribution);

  bind(FxdkProjectContribution).toSelf().inSingletonScope();
  bind(FrontendApplicationContribution).toService(FxdkProjectContribution);

  rebindApplicationShell(bind, rebind);
  rebindNavigator(bind, rebind);

  rebindWorkspacePreferences(rebind);
  rebindWorkspaceService(bind, rebind);
  rebindWorkspaceCommands(bind, rebind);
  rebindWorkspaceFrontendContribution(bind, rebind);

  bindViewContribution(bind, ServerConsoleViewContribution);
  bind(WidgetFactory).toDynamicValue((ctx) => ({
    id: SERVER_CONSOLE_WIDGET_ID,
    createWidget: () => ctx.container.resolve(ServerConsole),
  }));
});
