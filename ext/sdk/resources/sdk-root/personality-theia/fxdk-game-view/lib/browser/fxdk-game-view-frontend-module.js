"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fxdk_game_view_view_1 = require("./fxdk-game-view-view");
const inversify_1 = require("inversify");
const browser_1 = require("@theia/core/lib/browser");
exports.default = new inversify_1.ContainerModule(bind => {
    browser_1.bindViewContribution(bind, fxdk_game_view_view_1.FxdkGameViewContribution);
    bind(fxdk_game_view_view_1.FxdkGameView).toSelf();
    bind(browser_1.WidgetFactory).toDynamicValue(ctx => ({
        id: fxdk_game_view_view_1.FxdkGameView.ID,
        createWidget: () => ctx.container.get(fxdk_game_view_view_1.FxdkGameView)
    }));
});
//# sourceMappingURL=fxdk-game-view-frontend-module.js.map