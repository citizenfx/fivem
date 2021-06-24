import { injectable, inject, postConstruct } from 'inversify';

import { Widget, Message, AbstractViewContribution } from '@theia/core/lib/browser';
import { BaseWidget } from '@theia/core/lib/browser/widgets/widget';
import { ClientResourceData, FxdkDataService } from 'fxdk-services/lib/browser/fxdk-data-service';
import { formatMemory, formatMS, formatResourceName, formatTimePercentage } from './formatters';

export const RESMON_WIDGET_ID = 'fxdk-resmon';
export const RESMON_WIDGET_ICON = 'fa fa-dashboard';

interface ResourceNode {
  row: HTMLTableRowElement,

  clientCpu: HTMLTableCellElement,
  serverCpu: HTMLTableCellElement,

  clientTime: HTMLTableCellElement,
  serverTime: HTMLTableCellElement,

  clientMemory: HTMLTableCellElement,
  serverMemory: HTMLTableCellElement,

  streaming: HTMLTableCellElement,
}

@injectable()
export class ResmonWidget extends BaseWidget {
  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  protected tbodyNode: HTMLTableSectionElement | null;

  protected resourceNodes: Record<string, ResourceNode> = {};

  @postConstruct()
  protected init() {
    this.id = RESMON_WIDGET_ID;

    this.title.caption = 'Resource Monitor';
    this.title.label = 'Resource Monitor';
    this.title.iconClass = 'fa fa-dashboard';
    this.title.closable = true;

    this.addClass('fxdk-resmon-widget');

    this.scrollOptions = {};

    this.toDispose.pushAll([
      this.dataService.onClientResourcesData((data) => this.renderResourcesData(false, data)),
      this.dataService.onServerResourcesData((data) => this.renderResourcesData(true, data)),
    ]);
  }

  private renderResourcesData(server: boolean, data: ClientResourceData[]) {
    if (!this.initialized) {
      return;
    }

    const aliveResourceNames: Record<string, boolean> = Object.create(null);

    for (const [resourceName, cpu, time, memory, streaming] of data) {
      aliveResourceNames[resourceName] = true;

      const resourceNode = this.getOrCreateResourceNode(resourceName);

      if (server) {
        resourceNode.serverCpu.innerHTML = formatMS(cpu) + '';
        resourceNode.serverTime.innerHTML = formatTimePercentage(time) + '';
        resourceNode.serverMemory.innerHTML = formatMemory('+', memory) + '';
      } else {
        resourceNode.clientCpu.innerHTML = formatMS(cpu) + '';
        resourceNode.clientTime.innerHTML = formatTimePercentage(time) + '';
        resourceNode.clientMemory.innerHTML = formatMemory('+', memory) + '';
        resourceNode.streaming.innerHTML = formatMemory('', streaming) + '';
      }
    }

    for (const resourceName of Object.keys(this.resourceNodes)) {
      if (!aliveResourceNames[resourceName]) {
        this.deleteResourceNode(resourceName);
      }
    }
  }

  private getOrCreateResourceNode(resourceName: string) {
    if (!this.resourceNodes[resourceName]) {
      const row = document.createElement('tr');

      const name = document.createElement('td');
      name.classList.add('align-left');
      name.innerHTML = formatResourceName(resourceName) + '';

      const node: ResourceNode = this.resourceNodes[resourceName] = {
        row,
        clientCpu: document.createElement('td'),
        serverCpu: document.createElement('td'),
        clientTime: document.createElement('td'),
        serverTime: document.createElement('td'),
        clientMemory: document.createElement('td'),
        serverMemory: document.createElement('td'),
        streaming: document.createElement('td'),
      };

      node.clientCpu.classList.add('align-right');
      node.serverCpu.classList.add('align-right');
      node.clientTime.classList.add('align-right');
      node.serverTime.classList.add('align-right');
      node.clientMemory.classList.add('align-right');
      node.serverMemory.classList.add('align-right');
      node.streaming.classList.add('align-right');

      row.append(
        name,
        node.clientCpu,
        node.serverCpu,
        node.clientTime,
        node.serverTime,
        node.clientMemory,
        node.serverMemory,
        node.streaming,
      );

      this.tbodyNode?.appendChild(row);
    }

    return this.resourceNodes[resourceName];
  }

  private deleteResourceNode(resourceName: string) {
    const resourceNode = this.resourceNodes[resourceName];

    if (resourceNode) {
      this.tbodyNode?.removeChild(resourceNode.row);
      delete this.resourceNodes[resourceName];
    }
  }

  protected onActivateRequest(msg: Message): void {
    super.onActivateRequest(msg);
    this.node.focus();
  }

  protected onAfterShow(msg: Message): void {
    super.onAfterShow(msg);
    this.update();
  }

  protected onAfterAttach(msg: Message): void {
    super.onAfterAttach(msg);
    this.update();
  }

  protected onBeforeDetach(msg: Message): void {
    super.onBeforeDetach(msg);
  }

  protected onResize(msg: Widget.ResizeMessage): void {
    super.onResize(msg);
    this.update();
    this.updateScroll();
  }

  private initialized = false;
  protected onUpdateRequest(msg: Message): void {
    super.onUpdateRequest(msg);
    if (!this.isVisible || !this.isAttached) {
      return;
    }

    if (!this.initialized) {
      this.initialized = true;

      this.node.innerHTML = `
        <table cellpadding=0 cellspacing=0>
          <thead>
            <th class="align-left">Resource</th>
            <th class="align-right">Client CPU</th>
            <th class="align-right">Server CPU</th>
            <th class="align-right">Client Time %</th>
            <th class="align-right">Server Time %</th>
            <th class="align-right">Client Memory</th>
            <th class="align-right">Server Memory</th>
            <th class="align-right">Streaming</th>
          </thead>
          <tbody>
          </tbody>
        </table>
      `;

      this.tbodyNode = this.node.querySelector('tbody');

      this.renderResourcesData(false, this.dataService.getClientResourcesData());
      this.renderResourcesData(true, this.dataService.getServerResourcesData());
    }

    this.updateScroll();
  }

  private updateScroll() {
    this.scrollBar?.update();
  }
}

@injectable()
export class ResmonWidgetViewContribution extends AbstractViewContribution<ResmonWidget> {
  static readonly TOGGLE_COMMAND_ID = 'Z_fxdkResmon:toggle';

  constructor() {
    super({
      widgetId: RESMON_WIDGET_ID,
      widgetName: 'Resource Monitor',
      toggleCommandId: ResmonWidgetViewContribution.TOGGLE_COMMAND_ID,
      defaultWidgetOptions: {
        area: 'bottom',
      },
    });
  }
}
