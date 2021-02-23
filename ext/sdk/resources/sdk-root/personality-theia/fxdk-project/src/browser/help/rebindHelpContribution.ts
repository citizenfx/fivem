import { injectable, inject, interfaces } from 'inversify';
import { AbstractViewContribution, CommonCommands, CommonMenus, FrontendApplication, FrontendApplicationContribution } from '@theia/core/lib/browser';
import { GettingStartedWidget } from '@theia/getting-started/lib/browser/getting-started-widget';
import { GettingStartedContribution, GettingStartedCommand } from '@theia/getting-started/lib/browser/getting-started-contribution';
import { FxdkFAQWidget } from './fxdk-faq-widget';
import { FrontendApplicationStateService } from '@theia/core/lib/browser/frontend-application-state';
import { FxdkWorkspaceService } from '../rebindWorkspaceService';
import { Command, CommandRegistry, MenuModelRegistry } from '@theia/core';
import { FxdkDataService } from '../fxdk-data-service';

export namespace FxdkHelpCommands {
  export const DOCUMENTATION_LINK: Command = {
    id: 'fxdk.openDocumentation',
    label: 'FiveM Documentation',
  };
  export const NATIVES_LINK: Command = {
    id: 'fxdk.openNatives',
    label: 'FiveM Natives',
  };
  export const FORUMS_LINK: Command = {
    id: 'fxdk.openForums',
    label: 'Cfx.re Forums',
  };
}

@injectable()
export class FxdkHelpContribution extends AbstractViewContribution<GettingStartedWidget> implements FrontendApplicationContribution {
  @inject(FrontendApplicationStateService)
  protected readonly stateService: FrontendApplicationStateService;

  @inject(FxdkWorkspaceService)
  protected readonly workspaceService: FxdkWorkspaceService;

  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  constructor() {
    super({
      widgetId: GettingStartedWidget.ID,
      widgetName: 'FAQ',
      defaultWidgetOptions: {
        area: 'main',
      },
    });
  }

  async onStart(app: FrontendApplication): Promise<void> {
    if (!window.localStorage['fxdk-faq-shown']) {
      window.localStorage['fxdk-faq-shown'] = 'yes';

      this.stateService.reachedState('ready').then(
        () => this.openView({ reveal: true })
      );
    }
  }

  registerCommands(registry: CommandRegistry): void {
    registry.registerCommand(GettingStartedCommand, {
      execute: () => this.openView({ reveal: true }),
    });

    registry.registerCommand(FxdkHelpCommands.DOCUMENTATION_LINK, {
      execute: () => invokeNative('openUrl', 'https://docs.fivem.net/docs/'),
    });
    registry.registerCommand(FxdkHelpCommands.NATIVES_LINK, {
      execute: () => invokeNative('openUrl', 'https://docs.fivem.net/natives/'),
    });
    registry.registerCommand(FxdkHelpCommands.FORUMS_LINK, {
      execute: () => invokeNative('openUrl', 'https://forum.cfx.re/'),
    });
  }

  registerMenus(menus: MenuModelRegistry): void {
    menus.unregisterMenuAction({
      commandId: CommonCommands.ABOUT_COMMAND.id,
      label: 'About',
      order: '9',
    }, CommonMenus.HELP);

    menus.registerMenuAction(CommonMenus.HELP, {
      commandId: GettingStartedCommand.id,
      label: 'FAQ',
      order: 'a10',
      icon: 'fa fa-question',
    });
    menus.registerMenuAction(CommonMenus.HELP, {
      commandId: FxdkHelpCommands.DOCUMENTATION_LINK.id,
      label: 'FiveM Documentation',
      order: 'a11',
      icon: 'fa fa-book',
    });
    menus.registerMenuAction(CommonMenus.HELP, {
      commandId: FxdkHelpCommands.NATIVES_LINK.id,
      label: 'FiveM Natives',
      order: 'a12',
      icon: 'fa fa-code',
    });
    menus.registerMenuAction(CommonMenus.HELP, {
      commandId: FxdkHelpCommands.FORUMS_LINK.id,
      label: 'Cfx.re Forums',
      order: 'a13',
      icon: 'fa fa-users',
    });
  }
}

export function rebindHelpContribution(bind: interfaces.Bind, rebind: interfaces.Rebind) {
  bind(FxdkFAQWidget).toSelf();
  rebind(GettingStartedWidget).to(FxdkFAQWidget);

  bind(FxdkHelpContribution).toSelf().inSingletonScope();
  rebind(GettingStartedContribution).toService(FxdkHelpContribution as any);
}
