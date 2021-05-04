import { injectable, inject } from 'inversify';
import { BaseConsole } from './base-console';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { FxdkDataService } from 'fxdk-services/lib/browser/fxdk-data-service';

export const CLIENT_CONSOLE_WIDGET_ID = 'fxdk-client-console';
export const CLIENT_CONSOLE_WIDGET_ICON = 'fa fa-terminal';

@injectable()
export class ClientConsole extends BaseConsole {
  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  protected setup() {
    this.id = CLIENT_CONSOLE_WIDGET_ID;

    this.title.caption = 'Client console';
    this.title.label = 'Client console';
    this.title.iconClass = 'fa fa-terminal';
    this.title.closable = true;

    this.toDispose.push(this.dataService.onStructuredGameMessageReceived((message) => this.receiveStructuredMessage(message)));
    this.toDispose.push(this.dataService.onClearGameOutput(() => this.clear()));
  }

  protected initilizeConsole() {
    this.dataService.getStructuredGameMessage().forEach((message) => this.receiveStructuredMessage(message));
  }

  protected handleCommand(command: string) {
    (window as any).invokeNative('sendCommand', command);
  }
}

@injectable()
export class ClientConsoleViewContribution extends AbstractViewContribution<ClientConsole> {
  static readonly TOGGLE_COMMAND_ID = 'Z_fxdkClientConsole:toggle';

  constructor() {
    super({
      widgetId: CLIENT_CONSOLE_WIDGET_ID,
      widgetName: 'Server console',
      toggleCommandId: ClientConsoleViewContribution.TOGGLE_COMMAND_ID,
      defaultWidgetOptions: {
        area: 'bottom',
      }
    });
  }
}
