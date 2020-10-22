"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var inversify_1 = require("inversify");
var browser_1 = require("@theia/core/lib/browser");
var rebindWorkspacePreferences_1 = require("./rebindWorkspacePreferences");
var rebindWorkspaceService_1 = require("./rebindWorkspaceService");
var rebindWorkspaceCommands_1 = require("./rebindWorkspaceCommands");
var rebindWorkspaceFrontendContribution_1 = require("./rebindWorkspaceFrontendContribution");
var rebindApplicationShell_1 = require("./rebindApplicationShell");
var rebindNavigator_1 = require("./rebindNavigator");
var fxdk_project_contribution_1 = require("./fxdk-project-contribution");
var fxdk_data_service_1 = require("./fxdk-data-service");
var fxdk_menu_contribution_1 = require("./fxdk-menu-contribution");
var core_1 = require("@theia/core");
exports.default = new inversify_1.ContainerModule(function (bind, unbind, isBound, rebind) {
    bind(fxdk_data_service_1.FxdkDataService).toSelf().inSingletonScope();
    bind(fxdk_menu_contribution_1.FxdkMenuContribution).toSelf().inSingletonScope();
    bind(core_1.MenuContribution).toService(fxdk_menu_contribution_1.FxdkMenuContribution);
    bind(core_1.CommandContribution).toService(fxdk_menu_contribution_1.FxdkMenuContribution);
    bind(fxdk_project_contribution_1.FxdkProjectContribution).toSelf().inSingletonScope();
    bind(browser_1.FrontendApplicationContribution).toService(fxdk_project_contribution_1.FxdkProjectContribution);
    rebindApplicationShell_1.rebindApplicationShell(bind, rebind);
    rebindNavigator_1.rebindNavigator(bind, rebind);
    rebindWorkspacePreferences_1.rebindWorkspacePreferences(rebind);
    rebindWorkspaceService_1.rebindWorkspaceService(bind, rebind);
    rebindWorkspaceCommands_1.rebindWorkspaceCommands(bind, rebind);
    rebindWorkspaceFrontendContribution_1.rebindWorkspaceFrontendContribution(bind, rebind);
});
//# sourceMappingURL=fxdk-project-frontend-module.js.map