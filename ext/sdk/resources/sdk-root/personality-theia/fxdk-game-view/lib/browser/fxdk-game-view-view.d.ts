import * as React from 'react';
import { ReactWidget } from '@theia/core/lib/browser';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import './common/game-view.webcomponent.js';
export declare class FxdkGameView extends ReactWidget {
    static readonly ID = "fxdkGameView";
    init(): void;
    protected onActivateRequest(): void;
    protected render(): React.ReactNode;
}
export declare class FxdkGameViewContribution extends AbstractViewContribution<FxdkGameView> {
    static readonly FXDK_GAME_VIEW_TOGGLE_COMMAND_ID = "fxdkGameView:toggle";
    constructor();
}
//# sourceMappingURL=fxdk-game-view-view.d.ts.map