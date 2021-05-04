import { injectable, inject, postConstruct } from 'inversify';

import { Widget, Message } from '@theia/core/lib/browser';
import { BaseWidget } from '@theia/core/lib/browser/widgets/widget';
import { TerminalThemeService } from '@theia/terminal/lib/browser/terminal-theme-service';
import { TerminalPreferences } from '@theia/terminal/lib/browser/terminal-preferences';
import { colorizeString, rgbForKey } from '../utils/color';
import { Deferred } from '@theia/core/lib/common/promise-util';
import { FixedLengthBuffer } from '../utils/fixed-length-buffer';
import { Disposable } from '@theia/core';
import { StructuredMessage } from 'fxdk-services/lib/browser/fxdk-data-service';

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

  protected logNode: HTMLDivElement;
  protected outputNode: HTMLDivElement;
  protected outputStructuredNode: HTMLDivElement;
  protected inputNode: HTMLInputElement;

  protected scrollNodeDeferred = new Deferred();

  protected removeNodeQueue: HTMLElement[] = [];
  protected appendNodeQueue: HTMLElement[] = [];
  protected nodesBuffer = new FixedLengthBuffer<HTMLElement>(500);
  protected raf: number | null = null;

  protected abstract setup();

  @postConstruct()
  protected init() {
    this.setup();

    this.scrollOptions = {};

    this.addClass('fxdk-console-widget');

    this.logNode = document.createElement('div');
    this.outputNode = document.createElement('div');
    this.outputStructuredNode = document.createElement('div');
    this.inputNode = this.createInputNode();

    this.toDispose.push(this.preferences.onPreferenceChanged((change) => {
      if (change.preferenceName.indexOf('terminal.intergrated.') === -1) {
        return;
      }

      this.updateCssVar(change.preferenceName);
    }));

    this.toDispose.push(this.nodesBuffer);

    this.nodesBuffer.onRemove((removedNode) => {
      this.removeNodeQueue.push(removedNode);

      this.performRafActions();
    });

    this.toDispose.push(Disposable.create(() => {
      if (this.raf !== null) {
        cancelAnimationFrame(this.raf);
      }
    }));
  }

  protected receiveStructuredMessage({ channel, message }: StructuredMessage) {
    const [r, g, b] = rgbForKey(channel);
    const node = document.createElement('div');

    this.nodesBuffer.push(node);

    node.classList.add('structured-message');

    // Yes, ImGUI uses BGR but who would have known, right
    node.innerHTML = `
      <div class="channel" style="--channel-color: rgb(${b}, ${g}, ${r})">${channel}</div>
      <div class="message">${colorizeString(message)}</div>
    `;

    this.appendNodeQueue.push(node);

    this.performRafActions();
  }

  protected performRafActions() {
    if (this.raf === null ){
      this.raf = requestAnimationFrame(() => {
        this.raf = null;

        if (this.appendNodeQueue.length) {
          const appendNodes = this.appendNodeQueue;
          this.appendNodeQueue = [];
          this.outputStructuredNode.append(...appendNodes);
        }

        if (this.removeNodeQueue.length) {
          const removedNodes = this.removeNodeQueue;
          this.removeNodeQueue = [];
          removedNodes.forEach((removedNode) => {
            this.outputStructuredNode.removeChild(removedNode);
          });
        }

        this.updateScroll();
      });
    }
  }

  protected async getScrollContainer() {
    await this.scrollNodeDeferred.promise;
    return this.logNode;
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

    this.logNode.classList.add('log');
    this.logNode.prepend(this.outputNode, this.outputStructuredNode);
    this.node.prepend(this.logNode);

    const cmdInputNode = document.createElement('div');
    cmdInputNode.innerHTML = '<span>cmd:</span>';
    cmdInputNode.classList.add('cmd-input');
    cmdInputNode.appendChild(this.inputNode);

    this.node.prepend(cmdInputNode);

    console.log('Created console dom tree');
    this.scrollNodeDeferred.resolve();

    this.setupEventListeners();

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
      const node = this.logNode;
      node.scrollTop = node.scrollHeight - node.clientHeight;
    }
  }

  private setupEventListeners() {
    const mouseDownHandler = (e) => {
      // Handling right-click
      if (e.button === 2) {
        e.preventDefault();
        e.stopPropagation();

        const selection = window.getSelection();
        if (!selection) {
          return;
        }

        const selectionString = selection.toString();
        if (!selectionString) {
          return;
        }

        document.execCommand('copy');
        selection.empty();
      }
    };

    this.logNode.addEventListener('mousedown', mouseDownHandler);
    this.toDispose.push(Disposable.create(() => this.logNode.removeEventListener('mousedown', mouseDownHandler)));

    this.toDispose.push(this.onScrollUp(() => this.scrollOverride = true));
    this.toDispose.push(this.onScrollYReachEnd(() => this.scrollOverride = false));
  }
}
