import { injectable, interfaces } from 'inversify';

import { ApplicationShell } from '@theia/core/lib/browser/shell/application-shell';
import { DockPanel, SidePanel, Widget } from '@theia/core/lib/browser';

@injectable()
export class FxdkApplicationShell extends ApplicationShell {
  /**
     * Add a widget to the application shell. The given widget must have a unique `id` property,
     * which will be used as the DOM id.
     *
     * Widgets are removed from the shell by calling their `close` or `dispose` methods.
     *
     * Widgets added to the top area are not tracked regarding the _current_ and _active_ states.
     */
  async addWidget(widget: Widget, options: Readonly<ApplicationShell.WidgetOptions> = {}): Promise<void> {
    if (!widget.id) {
      console.error('Widgets added to the application shell must have a unique id property.');
      return;
    }
    let ref: Widget | undefined = options.ref;
    let area: ApplicationShell.Area = options.area || 'main';
    if (!ref && (area === 'main' || area === 'bottom')) {
      const tabBar = this.getTabBarFor(area);
      ref = tabBar && tabBar.currentTitle && tabBar.currentTitle.owner || undefined;
    }
    // make sure that ref belongs to area
    area = ref && this.getAreaFor(ref) || area;
    const addOptions: DockPanel.IAddOptions = {};
    if (ApplicationShell.isOpenToSideMode(options.mode)) {
      const areaPanel = area === 'main' ? this.mainPanel : area === 'bottom' ? this.bottomPanel : undefined;
      const sideRef = areaPanel && ref && (options.mode === 'open-to-left' ?
        areaPanel.previousTabBarWidget(ref) :
        areaPanel.nextTabBarWidget(ref));
      if (sideRef) {
        addOptions.ref = sideRef;
      } else {
        addOptions.ref = ref;
        addOptions.mode = options.mode === 'open-to-left' ? 'split-left' : 'split-right';
      }
    } else {
      addOptions.ref = ref;
      addOptions.mode = options.mode;
    }
    const sidePanelOptions: SidePanel.WidgetOptions = { rank: options.rank };
    switch (area) {
      case 'main':
        this.mainPanel.addWidget(widget, addOptions);
        break;
      case 'top':
        this.topPanel.addWidget(widget);
        break;
      case 'bottom':
        this.bottomPanel.addWidget(widget, addOptions);
        break;
      case 'left':
        // Always add to the right panel
        this.rightPanelHandler.addWidget(widget, sidePanelOptions);
        break;
      case 'right':
        this.rightPanelHandler.addWidget(widget, sidePanelOptions);
        break;
      default:
        throw new Error('Unexpected area: ' + options.area);
    }
    if (area !== 'top') {
      this.track(widget);
    }
  }
}

export const rebindApplicationShell = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  bind(FxdkApplicationShell).toSelf().inSingletonScope();
  rebind(ApplicationShell).toService(FxdkApplicationShell);
};
