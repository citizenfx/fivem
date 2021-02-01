import { injectable, inject, postConstruct } from 'inversify';

import { Widget, Message } from '@theia/core/lib/browser';
import { BaseWidget } from '@theia/core/lib/browser/widgets/widget';
import { TerminalThemeService } from '@theia/terminal/lib/browser/terminal-theme-service';
import { TerminalPreferences } from '@theia/terminal/lib/browser/terminal-preferences';
import { StructuredMessage } from '../fxdk-data-service';
import { colorizeString, rgbForKey } from '../utils/color';

const asIs = x => x;
const asPx = x => `${x}px`;

const preferenceIdToCss = {
  'terminal.integrated.fontFamily': { id: '--fontFamily', transform: asIs },
  'terminal.integrated.fontSize': { id: '--fontSize', transform: asPx },
  'terminal.integrated.fontWeight': { id: '--fontWeight', transform: asIs },
  'terminal.integrated.letterSpacing': { id: '--letterSpacing', transform: asPx },
  'terminal.integrated.lineHeight': { id: '--lineHeight', transform: asIs },
};

export const SERVER_TERMINAL_WIDGET_ID = 'fxdk-server-terminal';

@injectable()
export abstract class BaseConsole extends BaseWidget {
  @inject(TerminalThemeService)
  protected readonly themeService: TerminalThemeService;

  @inject(TerminalPreferences)
  protected readonly preferences: TerminalPreferences;

  protected outputNode: HTMLDivElement;
  protected outputStructuredNode: HTMLDivElement;
  protected inputNode: HTMLInputElement;

  protected abstract setup();

  @postConstruct()
  protected init() {
    this.setup();

    this.scrollOptions = {};

    this.addClass('fxdk-console-widget');

    this.outputNode = document.createElement('div');
    this.outputStructuredNode = document.createElement('div');
    this.inputNode = this.createInputNode();

    this.toDispose.push(this.preferences.onPreferenceChanged((change) => {
      if (change.preferenceName.indexOf('terminal.intergrated.') === -1) {
        return;
      }

      this.updateCssVar(change.preferenceName);
    }));
  }

  protected abstract handleCommand(command: string);
  protected createInputNode(): HTMLInputElement {
    const node = document.createElement('input');

    node.addEventListener('keydown', (event) => {
      if (event.key === 'Enter') {
        const command = node.value.trim();
        if (command) {
          node.value = '';
          this.handleCommand(command);
        }
      }
    });

    return node;
  }

  protected updateCssVar(preferenceId: string) {
    const cfg = preferenceIdToCss[preferenceId];
    if (!cfg) {
      return;
    }

    this.node.style.setProperty(cfg.id, cfg.transform(this.preferences[preferenceId]));
  }

  protected populateCssVars() {
    Object.entries(preferenceIdToCss).forEach(([preferenceId, cfg]) => {
      this.node.style.setProperty(cfg.id, cfg.transform(this.preferences[preferenceId]));
    });
  }

  protected setBufferedOutput(output: string) {
    requestAnimationFrame(() => {
      this.outputNode.innerHTML = output.split('\n').join('\n<br/>\n');
      this.updateScroll();
    });
  }

  protected nodeQueue: HTMLElement[] = [];
  protected raf: number | null = null;
  protected receiveStructuredMessage({ channel, message }: StructuredMessage) {
    const [r, g, b] = rgbForKey(channel);
    const node = document.createElement('div');

    node.classList.add('structured-message');

    // Yes, ImGUI uses BGR but who would have known, right
    node.innerHTML = `
      <div class="channel" style="--channel-color: rgb(${b}, ${g}, ${r})">${channel}</div>
      <div class="message">${colorizeString(message)}</div>
    `;

    this.nodeQueue.push(node);

    if (this.raf === null ){
      this.raf = requestAnimationFrame(() => {
        this.raf = null;
        const nodes = this.nodeQueue;
        this.nodeQueue = [];
        this.outputStructuredNode.append(...nodes);
        this.updateScroll();
      });
    }
  }

  protected clear() {
    this.outputNode.innerHTML = '';
    this.outputStructuredNode.innerHTML = '';
    this.scrollBar?.update();
  }

  protected onActivateRequest(msg: Message): void {
    super.onActivateRequest(msg);
    this.inputNode.focus();
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

  protected consoleInitialized = false;
  protected abstract initilizeConsole();
  protected open(): void {
    if (this.consoleInitialized) {
      return;
    }

    this.consoleInitialized = true;

    this.populateCssVars();

    this.setupScrollListeners();

    this.node.appendChild(this.outputNode);
    this.node.appendChild(this.outputStructuredNode);

    const labelNode = document.createElement('div');
    labelNode.innerHTML = '<span>cmd:</span>';
    labelNode.classList.add('label');
    labelNode.appendChild(this.inputNode);

    this.node.appendChild(labelNode);

    this.initilizeConsole();
  }

  protected needsResize = true;
  protected onUpdateRequest(msg: Message): void {
    super.onUpdateRequest(msg);
    if (!this.isVisible || !this.isAttached) {
      return;
    }

    this.open();
    this.updateScroll();
  }

  protected scrollOverride = false;
  private updateScroll() {
    this.scrollBar?.update();

    if (!this.scrollOverride) {
      this.node.scrollTop = this.node.scrollHeight - this.node.clientHeight;
    }
  }

  private setupScrollListeners() {
    this.toDispose.push(this.onScrollUp(() => this.scrollOverride = true));
    this.toDispose.push(this.onScrollYReachEnd(() => this.scrollOverride = false));
  }
}
