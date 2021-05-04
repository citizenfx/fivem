import { injectable, postConstruct } from 'inversify';

import { Widget, Message } from '@theia/core/lib/browser';
import { BaseWidget } from '@theia/core/lib/browser/widgets/widget';

export interface RowDef {
  title: string,
  format?(cell: unknown): unknown,
  align?: 'left' | 'center' | 'right',
}

@injectable()
export abstract class BaseResmon extends BaseWidget {
  protected rowDefs: RowDef[] = [];

  protected tbodyNode: HTMLTableSectionElement | null;

  protected abstract setup();
  protected abstract initializeResmon();

  @postConstruct()
  protected init() {
    this.setup();

    this.addClass('fxdk-resmon-widget');

    this.scrollOptions = {};
  }

  protected renderRows(rows: unknown[][]) {
    if (!this.initialized) {
      return;
    }

    if (this.tbodyNode) {
      this.tbodyNode.innerHTML = rows.map((row) => {
        return `<tr>${row.map((cell, i) => this.renderCell(i, cell)).join('')}</tr>`;
      }).join('');
    }
  }

  private renderCell(i: number, cell: unknown) {
    const formatted = this.formatCell(i, cell);
    const align = this.rowDefs[i]?.align || 'center';

    return `<td class="align-${align}">${formatted}</td>`;
  }

  private formatCell(i: number, cell: unknown) {
    const formatter = this.rowDefs[i]?.format;

    if (formatter) {
      return formatter(cell);
    }

    return cell;
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
            ${this.rowDefs.map((_, i) => this.renderHeader(i)).join('')}
          </thead>
          <tbody>
          </tbody>
        </table>
      `;

      this.tbodyNode = this.node.querySelector('tbody');

      this.initializeResmon();
    }

    this.updateScroll();
  }

  private renderHeader(i: number) {
    const { title, align = 'center' } = this.rowDefs[i];

    return `<th class="align-${align}">${title}</th>`;
  }

  private updateScroll() {
    this.scrollBar?.update();
  }
}
