"use strict";
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.FxdkProjectContribution = void 0;
var inversify_1 = require("inversify");
var browser_1 = require("@theia/workspace/lib/browser");
var browser_2 = require("@theia/core/lib/browser");
var rebindWorkspaceService_1 = require("./rebindWorkspaceService");
var uri_1 = require("@theia/core/lib/common/uri");
var core_1 = require("@theia/core");
var fxdk_data_service_1 = require("./fxdk-data-service");
var fxdk_game_view_view_1 = require("fxdk-game-view/lib/browser/fxdk-game-view-view");
var frontend_application_state_1 = require("@theia/core/lib/browser/frontend-application-state");
var stateToNumber = {
    init: 0,
    started_contributions: 1,
    attached_shell: 2,
    initialized_layout: 3,
    ready: 4,
    closing_window: 5,
};
function mapStateToNumber(state) {
    return stateToNumber[state];
}
function mapFxdkFolderToTheiaFolder(folder) {
    return folder.replace(/\\/g, '/');
}
var FxdkProjectContribution = /** @class */ (function () {
    function FxdkProjectContribution() {
    }
    FxdkProjectContribution.prototype.initialize = function () {
        var _this = this;
        window.parent.postMessage({ type: 'theia:ready' }, '*');
        window.addEventListener('message', function (e) {
            if (typeof e.data !== 'object' || e.data === null) {
                return;
            }
            var _a = e.data, type = _a.type, data = _a.data;
            if (type.indexOf('fxdk:') !== 0) {
                return;
            }
            var methodBase = type.substr(5);
            var method = 'handle' + methodBase[0].toUpperCase() + methodBase.substr(1);
            if (typeof _this[method] !== 'undefined') {
                return _this[method](data);
            }
        });
        document.addEventListener('contextmenu', function (e) { return e.preventDefault(); });
    };
    FxdkProjectContribution.prototype.handleForceReload = function () {
        this.sendMessageToShell('theia:notReady');
        return window.location.reload();
    };
    FxdkProjectContribution.prototype.handleSetProject = function (project) {
        console.log('setting or updating project', project);
        var theiaProject = {
            name: project.name,
            path: project.path,
            folders: project.folders.map(mapFxdkFolderToTheiaFolder),
        };
        return this.fxdkWorkspaceService.setProject(theiaProject);
    };
    FxdkProjectContribution.prototype.handleSetFolders = function (folders) {
        console.log('setting folders', folders);
        return this.fxdkWorkspaceService.setFolders(folders.map(mapFxdkFolderToTheiaFolder));
    };
    FxdkProjectContribution.prototype.handleAddFolders = function (folders) {
        console.log('adding folders', folders);
        return this.fxdkWorkspaceService.addFolders(folders.map(mapFxdkFolderToTheiaFolder));
    };
    FxdkProjectContribution.prototype.handleOpenFile = function (file) {
        if (!this.reachedState('ready')) {
            return;
        }
        var lastIndexOfBackslash = file.lastIndexOf('\\');
        var baseUri = new uri_1.default('file:///' + file.substr(0, lastIndexOfBackslash).replace(/\\/g, '/'));
        var fileName = file.substr(lastIndexOfBackslash + 1);
        var uri = baseUri.resolve(fileName);
        return browser_2.open(this.openService, uri);
    };
    FxdkProjectContribution.prototype.handleData = function (data) {
        var _this = this;
        if (!this.reachedState('ready')) {
            return;
        }
        if (Array.isArray(data)) {
            data.forEach(function (_a) {
                var key = _a.key, value = _a.value;
                _this.dataService.data[key] = value;
            });
        }
        else {
            this.dataService.data[data.key] = data.value;
        }
    };
    FxdkProjectContribution.prototype.handleOpenGameView = function () {
        if (!this.reachedState('ready')) {
            return;
        }
        if (!this.widgetManager.tryGetWidget(fxdk_game_view_view_1.FxdkGameView.ID)) {
            console.log('Opening Game View');
            this.commandService.executeCommand(fxdk_game_view_view_1.FxdkGameViewContribution.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID);
        }
    };
    FxdkProjectContribution.prototype.reachedState = function (state) {
        var stateNumber = mapStateToNumber(state);
        var currentStateNumber = mapStateToNumber(this.stateService.state);
        return currentStateNumber >= stateNumber;
    };
    FxdkProjectContribution.prototype.sendMessageToShell = function (type, data) {
        window.parent.postMessage({ type: type, data: data }, '*');
    };
    __decorate([
        inversify_1.inject(core_1.CommandService),
        __metadata("design:type", Object)
    ], FxdkProjectContribution.prototype, "commandService", void 0);
    __decorate([
        inversify_1.inject(browser_1.WorkspaceService),
        __metadata("design:type", rebindWorkspaceService_1.FxdkWorkspaceService)
    ], FxdkProjectContribution.prototype, "fxdkWorkspaceService", void 0);
    __decorate([
        inversify_1.inject(browser_2.OpenerService),
        __metadata("design:type", Object)
    ], FxdkProjectContribution.prototype, "openService", void 0);
    __decorate([
        inversify_1.inject(fxdk_data_service_1.FxdkDataService),
        __metadata("design:type", fxdk_data_service_1.FxdkDataService)
    ], FxdkProjectContribution.prototype, "dataService", void 0);
    __decorate([
        inversify_1.inject(browser_2.WidgetManager),
        __metadata("design:type", browser_2.WidgetManager)
    ], FxdkProjectContribution.prototype, "widgetManager", void 0);
    __decorate([
        inversify_1.inject(frontend_application_state_1.FrontendApplicationStateService),
        __metadata("design:type", frontend_application_state_1.FrontendApplicationStateService)
    ], FxdkProjectContribution.prototype, "stateService", void 0);
    FxdkProjectContribution = __decorate([
        inversify_1.injectable()
    ], FxdkProjectContribution);
    return FxdkProjectContribution;
}());
exports.FxdkProjectContribution = FxdkProjectContribution;
//# sourceMappingURL=fxdk-project-contribution.js.map