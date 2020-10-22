import { interfaces } from 'inversify';
import { ApplicationShell } from '@theia/core/lib/browser/shell/application-shell';
import { Widget } from '@theia/core/lib/browser';
export declare class FxdkApplicationShell extends ApplicationShell {
    /**
       * Add a widget to the application shell. The given widget must have a unique `id` property,
       * which will be used as the DOM id.
       *
       * Widgets are removed from the shell by calling their `close` or `dispose` methods.
       *
       * Widgets added to the top area are not tracked regarding the _current_ and _active_ states.
       */
    addWidget(widget: Widget, options?: Readonly<ApplicationShell.WidgetOptions>): Promise<void>;
}
export declare const rebindApplicationShell: (bind: interfaces.Bind, rebind: interfaces.Rebind) => void;
//# sourceMappingURL=rebindApplicationShell.d.ts.map