import { injectable, inject } from 'inversify';
import { BaseConsole } from './base-console';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { FxdkDataService } from '../fxdk-data-service';

export const SERVER_CONSOLE_WIDGET_ID = 'fxdk-server-console';
export const SERVER_CONSOLE_WIDGET_ICON = 'fa fa-terminal';

@injectable()
export class ServerConsole extends BaseConsole {
  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  protected setup() {
    this.id = SERVER_CONSOLE_WIDGET_ID;

    this.title.caption = 'Server console';
    this.title.label = 'Server console';
    this.title.iconClass = 'fa fa-terminal';
    this.title.closable = true;

    this.toDispose.push(this.dataService.onBufferedServerOutputChanged(() => this.setBufferedOutput(this.dataService.getBufferedServerOutput())));
    this.toDispose.push(this.dataService.onStructuredServerMessageReceived((message) => this.receiveStructuredMessage(message)));
    this.toDispose.push(this.dataService.onClearAllServerOutputs(() => this.clear()));
  }

  protected initilizeConsole() {
    this.setBufferedOutput(this.dataService.getBufferedServerOutput());
    this.dataService.getStructuredServerMessages().forEach((message) => this.receiveStructuredMessage(message));
  }

  protected handleCommand(command: string) {
    this.dataService.sendMessageToShell('server:sendCommand', command);
  }
}

@injectable()
export class ServerConsoleViewContribution extends AbstractViewContribution<ServerConsole> {
  static readonly TOGGLE_COMMAND_ID = 'Z_fxdkServerConsole:toggle';

  constructor() {
    super({
      widgetId: SERVER_CONSOLE_WIDGET_ID,
      widgetName: 'Server console',
      toggleCommandId: ServerConsoleViewContribution.TOGGLE_COMMAND_ID,
      defaultWidgetOptions: {
        area: 'bottom',
      }
    });
  }
}
