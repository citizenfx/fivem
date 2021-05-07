import { AbstractViewContribution } from '@theia/core/lib/browser';
import { FxdkDataService } from 'fxdk-services/lib/browser/fxdk-data-service';
import { injectable, inject } from 'inversify';
import { rgbForKey } from '../utils/color';
import { BaseResmon } from './base-resmon';

export const CLIENT_RESMON_WIDGET_ID = 'fxdk-client-resmon';
export const CLIENT_RESMON_WIDGET_ICON = 'fa fa-dashboard';

function f(n: number): string {
  return n.toFixed(2);
}

function formatMemory(mem: unknown): unknown {
  if (typeof mem === 'number') {
    if (mem <= 0) {
      return '?';
    }

    switch (true) {
      case mem >= (1024 ** 3): {
        return f(mem / (1024 ** 3)) + ' GiB';
      }
      case mem >= (1024 ** 2): {
        return f(mem / (1024 ** 2)) + ' MiB';
      }
      case mem >= 1024: {
        return f(mem / 1024) + ' KiB';
      }
      default: {
        return f(mem) + ' B';
      }
    }
  }

  return mem;
}

@injectable()
export class ClientResmon extends BaseResmon {
  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  protected setup() {
    this.id = CLIENT_RESMON_WIDGET_ID;

    this.title.caption = 'Client Resource Monitor';
    this.title.label = 'Client Resource Monitor';
    this.title.iconClass = 'fa fa-dashboard';
    this.title.closable = true;

    this.rowDefs = [
      {
        title: 'Resource',
        align: 'left',
        format(cell) {
          if (typeof cell === 'string') {
            const [r, g, b] = rgbForKey(cell);

            return `<span class=resource-name style="background:rgb(${r}, ${g}, ${b})">${cell}</span>`;
          }

          return cell;
        },
      },
      {
        title: 'CPU ms',
        align: 'right',
        format(cell) {
          if (typeof cell === 'number') {
            if (cell < 0) {
              return '-';
            }

            return cell.toFixed(2) + ' ms';
          }

          return cell;
        },
      },
      {
        title: 'Time %',
        align: 'right',
        format(cell) {
          if (typeof cell === 'number') {
            if (cell < 0) {
              return '-';
            }

            const intensityColor = `rgba(244, 5, 82, ${cell})`;

            return `<span style="padding: 0 2px;border: solid 2px ${intensityColor}">${(cell * 100).toFixed(2)}%</span>`;
          }

          return cell;
        },
      },
      {
        title: 'Memory',
        align: 'right',
        format: formatMemory,
      },
      {
        title: 'Streaming',
        align: 'right',
        format: formatMemory,
      },
    ];

    this.toDispose.push(this.dataService.onClientResourcesData((data) => this.renderRows(data)));
  }

  protected initializeResmon() {
    this.renderRows(this.dataService.getClientResourcesData());
  }
}

@injectable()
export class ClientResmonViewContribution extends AbstractViewContribution<ClientResmon> {
  static readonly TOGGLE_COMMAND_ID = 'Z_fxdkClientResmon:toggle';

  constructor() {
    super({
      widgetId: CLIENT_RESMON_WIDGET_ID,
      widgetName: 'Client Resource Monitor',
      toggleCommandId: ClientResmonViewContribution.TOGGLE_COMMAND_ID,
      defaultWidgetOptions: {
        area: 'bottom',
      }
    });
  }
}
