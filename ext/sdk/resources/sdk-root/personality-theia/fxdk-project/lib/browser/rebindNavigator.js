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
var __param = (this && this.__param) || function (paramIndex, decorator) {
    return function (target, key) { decorator(target, key, paramIndex); }
};
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var __read = (this && this.__read) || function (o, n) {
    var m = typeof Symbol === "function" && o[Symbol.iterator];
    if (!m) return o;
    var i = m.call(o), r, ar = [], e;
    try {
        while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
    }
    catch (error) { e = { error: error }; }
    finally {
        try {
            if (r && !r.done && (m = i["return"])) m.call(i);
        }
        finally { if (e) throw e.error; }
    }
    return ar;
};
var __spread = (this && this.__spread) || function () {
    for (var ar = [], i = 0; i < arguments.length; i++) ar = ar.concat(__read(arguments[i]));
    return ar;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.rebindNavigator = exports.FxdkFileNavigatorContribution = void 0;
var inversify_1 = require("inversify");
// import { AbstractViewContribution } from '@theia/core/lib/browser/shell/view-contribution';
var browser_1 = require("@theia/core/lib/browser");
// import { FileDownloadCommands } from '@theia/filesystem/lib/browser/download/file-download-command-contribution';
var common_1 = require("@theia/core/lib/common");
var browser_2 = require("@theia/workspace/lib/browser");
var tab_bar_toolbar_1 = require("@theia/core/lib/browser/shell/tab-bar-toolbar");
// import { FileSystemCommands } from '@theia/filesystem/lib/browser/filesystem-frontend-contribution';
// import { UriSelection } from '@theia/core/lib/common/selection';
var clipboard_service_1 = require("@theia/core/lib/browser/clipboard-service");
var selection_service_1 = require("@theia/core/lib/common/selection-service");
var navigator_widget_1 = require("@theia/navigator/lib/browser/navigator-widget");
var navigator_preferences_1 = require("@theia/navigator/lib/browser/navigator-preferences");
// import { NavigatorKeybindingContexts } from '@theia/navigator/lib/browser/navigator-keybinding-context';
var navigator_filter_1 = require("@theia/navigator/lib/browser/navigator-filter");
// import { WorkspaceNode } from '@theia/navigator/lib/browser/navigator-tree';
var navigator_context_key_service_1 = require("@theia/navigator/lib/browser/navigator-context-key-service");
var navigator_diff_1 = require("@theia/navigator/lib/browser/navigator-diff");
// import { FileNavigatorModel } from '@theia/navigator/lib/browser/navigator-model';
var navigator_contribution_1 = require("@theia/navigator/lib/browser/navigator-contribution");
var FxdkFileNavigatorContribution = /** @class */ (function () {
    function FxdkFileNavigatorContribution(fileNavigatorPreferences, openerService, fileNavigatorFilter, workspaceService, workspacePreferences) {
        var _this = this;
        this.fileNavigatorPreferences = fileNavigatorPreferences;
        this.openerService = openerService;
        this.fileNavigatorFilter = fileNavigatorFilter;
        this.workspaceService = workspaceService;
        this.workspacePreferences = workspacePreferences;
        /**
         * Register commands to the `More Actions...` navigator toolbar item.
         */
        this.registerMoreToolbarItem = function (item) {
            var commandId = item.command;
            var id = 'navigator.tabbar.toolbar.' + commandId;
            var command = _this.commandRegistry.getCommand(commandId);
            _this.commandRegistry.registerCommand({ id: id, iconClass: command && command.iconClass }, {
                execute: function (w) {
                    var _a;
                    var args = [];
                    for (var _i = 1; _i < arguments.length; _i++) {
                        args[_i - 1] = arguments[_i];
                    }
                    return w instanceof navigator_widget_1.FileNavigatorWidget
                        && (_a = _this.commandRegistry).executeCommand.apply(_a, __spread([commandId], args));
                },
                isEnabled: function (w) {
                    var _a;
                    var args = [];
                    for (var _i = 1; _i < arguments.length; _i++) {
                        args[_i - 1] = arguments[_i];
                    }
                    return w instanceof navigator_widget_1.FileNavigatorWidget
                        && (_a = _this.commandRegistry).isEnabled.apply(_a, __spread([commandId], args));
                },
                isVisible: function (w) {
                    var _a;
                    var args = [];
                    for (var _i = 1; _i < arguments.length; _i++) {
                        args[_i - 1] = arguments[_i];
                    }
                    return w instanceof navigator_widget_1.FileNavigatorWidget
                        && (_a = _this.commandRegistry).isVisible.apply(_a, __spread([commandId], args));
                },
                isToggled: function (w) {
                    var _a;
                    var args = [];
                    for (var _i = 1; _i < arguments.length; _i++) {
                        args[_i - 1] = arguments[_i];
                    }
                    return w instanceof navigator_widget_1.FileNavigatorWidget
                        && (_a = _this.commandRegistry).isToggled.apply(_a, __spread([commandId], args));
                },
            });
            item.command = id;
            _this.tabbarToolbarRegistry.registerItem(item);
        };
        this.toDisposeAddRemoveFolderActions = new common_1.DisposableCollection();
        // super({
        //   viewContainerId: EXPLORER_VIEW_CONTAINER_ID,
        //   widgetId: FILE_NAVIGATOR_ID,
        //   widgetName: 'Explorer',
        //   defaultWidgetOptions: {
        //     // area: 'left',
        //     // rank: 100
        //   },
        //   // toggleCommandId: 'fileNavigator:toggle',
        //   // toggleKeybinding: 'ctrlcmd+shift+e'
        // });
    }
    FxdkFileNavigatorContribution.prototype.init = function () {
        return __awaiter(this, void 0, void 0, function () {
            var _this = this;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.fileNavigatorPreferences.ready];
                    case 1:
                        _a.sent();
                        // this.shell.currentChanged.connect(() => this.onCurrentWidgetChangedHandler());
                        // const updateFocusContextKeys = () => {
                        //   // const hasFocus = this.shell.activeWidget instanceof FileNavigatorWidget;
                        //   // this.contextKeyService.explorerViewletFocus.set(hasFocus);
                        //   // this.contextKeyService.filesExplorerFocus.set(hasFocus);
                        // };
                        // updateFocusContextKeys();
                        // this.shell.activeChanged.connect(updateFocusContextKeys);
                        this.workspaceCommandContribution.onDidCreateNewFile(function (event) { return __awaiter(_this, void 0, void 0, function () { return __generator(this, function (_a) {
                            return [2 /*return*/, this.onDidCreateNewResource(event)];
                        }); }); });
                        this.workspaceCommandContribution.onDidCreateNewFolder(function (event) { return __awaiter(_this, void 0, void 0, function () { return __generator(this, function (_a) {
                            return [2 /*return*/, this.onDidCreateNewResource(event)];
                        }); }); });
                        return [2 /*return*/];
                }
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.onDidCreateNewResource = function (event) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.onStart = function (app) {
        return __awaiter(this, void 0, void 0, function () {
            var _this = this;
            return __generator(this, function (_a) {
                this.workspacePreferences.ready.then(function () {
                    _this.updateAddRemoveFolderActions(_this.menuRegistry);
                    _this.workspacePreferences.onPreferenceChanged(function (change) {
                        if (change.preferenceName === 'workspace.supportMultiRootWorkspace') {
                            _this.updateAddRemoveFolderActions(_this.menuRegistry);
                        }
                    });
                });
                return [2 /*return*/];
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.initializeLayout = function (app) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.registerCommands = function (registry) {
        // super.registerCommands(registry);
        return;
        // registry.registerCommand(FileNavigatorCommands.FOCUS, {
        //   execute: () => this.openView({ activate: true })
        // });
        // registry.registerCommand(FileNavigatorCommands.REVEAL_IN_NAVIGATOR, {
        //   execute: () => this.openView({ activate: true }).then(() => this.selectWidgetFileNode(this.shell.currentWidget)),
        //   isEnabled: () => Navigatable.is(this.shell.currentWidget),
        //   isVisible: () => Navigatable.is(this.shell.currentWidget)
        // });
        // registry.registerCommand(FileNavigatorCommands.TOGGLE_HIDDEN_FILES, {
        //   execute: () => {
        //     this.fileNavigatorFilter.toggleHiddenFiles();
        //   },
        //   isEnabled: () => true,
        //   isVisible: () => true
        // });
        // registry.registerCommand(FileNavigatorCommands.TOGGLE_AUTO_REVEAL, {
        //   isEnabled: widget => this.withWidget(widget, () => this.workspaceService.opened),
        //   isVisible: widget => this.withWidget(widget, () => this.workspaceService.opened),
        //   execute: () => {
        //     const autoReveal = !this.fileNavigatorPreferences['explorer.autoReveal'];
        //     this.preferenceService.set('explorer.autoReveal', autoReveal, PreferenceScope.User);
        //     if (autoReveal) {
        //       this.selectWidgetFileNode(this.shell.currentWidget);
        //     }
        //   },
        //   isToggled: () => this.fileNavigatorPreferences['explorer.autoReveal']
        // });
        // registry.registerCommand(FileNavigatorCommands.COLLAPSE_ALL, {
        //   execute: widget => this.withWidget(widget, () => this.collapseFileNavigatorTree()),
        //   isEnabled: widget => this.withWidget(widget, () => this.workspaceService.opened),
        //   isVisible: widget => this.withWidget(widget, () => this.workspaceService.opened)
        // });
        // registry.registerCommand(FileNavigatorCommands.REFRESH_NAVIGATOR, {
        //   execute: widget => this.withWidget(widget, () => this.refreshWorkspace()),
        //   isEnabled: widget => this.withWidget(widget, () => this.workspaceService.opened),
        //   isVisible: widget => this.withWidget(widget, () => this.workspaceService.opened)
        // });
        // registry.registerCommand(FileNavigatorCommands.ADD_ROOT_FOLDER, {
        //   execute: (...args) => registry.executeCommand(WorkspaceCommands.ADD_FOLDER.id, ...args),
        //   isEnabled: (...args) => registry.isEnabled(WorkspaceCommands.ADD_FOLDER.id, ...args),
        //   isVisible: (...args) => {
        //     if (!registry.isVisible(WorkspaceCommands.ADD_FOLDER.id, ...args)) {
        //       return false;
        //     }
        //     const navigator = this.tryGetWidget();
        //     const model = navigator && navigator.model;
        //     const uris = UriSelection.getUris(model && model.selectedNodes);
        //     return this.workspaceService.areWorkspaceRoots(uris);
        //   }
        // });
        // registry.registerCommand(NavigatorDiffCommands.COMPARE_FIRST, {
        //   execute: () => {
        //     this.navigatorDiff.addFirstComparisonFile();
        //   },
        //   isEnabled: () => true,
        //   isVisible: () => true
        // });
        // registry.registerCommand(NavigatorDiffCommands.COMPARE_SECOND, {
        //   execute: () => {
        //     this.navigatorDiff.compareFiles();
        //   },
        //   isEnabled: () => this.navigatorDiff.isFirstFileSelected,
        //   isVisible: () => this.navigatorDiff.isFirstFileSelected
        // });
        // registry.registerCommand(FileNavigatorCommands.COPY_RELATIVE_FILE_PATH, new UriAwareCommandHandler<URI[]>(this.selectionService, {
        //   isEnabled: uris => !!uris.length,
        //   isVisible: uris => !!uris.length,
        //   execute: async uris => {
        //     const lineDelimiter = isWindows ? '\r\n' : '\n';
        //     const text = uris.map((uri: URI) => {
        //       const workspaceRoot = this.workspaceService.getWorkspaceRootUri(uri);
        //       if (workspaceRoot) {
        //         return workspaceRoot.relative(uri);
        //       }
        //     }).join(lineDelimiter);
        //     await this.clipboardService.writeText(text);
        //   }
        // }, { multi: true }));
        // registry.registerCommand(FileNavigatorCommands.OPEN, {
        //   isEnabled: () => this.getSelectedFileNodes().length > 0,
        //   isVisible: () => this.getSelectedFileNodes().length > 0,
        //   execute: () => {
        //     this.getSelectedFileNodes().forEach(async node => {
        //       const opener = await this.openerService.getOpener(node.uri);
        //       opener.open(node.uri);
        //     });
        //   }
        // });
    };
    FxdkFileNavigatorContribution.prototype.getSelectedFileNodes = function () {
        return [];
        // return this.tryGetWidget()?.model.selectedNodes.filter(FileNode.is) || [];
    };
    // protected withWidget<T>(widget: Widget | undefined = this.tryGetWidget(), cb: (navigator: FileNavigatorWidget) => T): T | false {
    //   if (widget instanceof FileNavigatorWidget && widget.id === FILE_NAVIGATOR_ID) {
    //     return cb(widget);
    //   }
    //   return false;
    // }
    FxdkFileNavigatorContribution.prototype.registerMenus = function (registry) {
        // super.registerMenus(registry);
        return;
        // registry.registerMenuAction(SHELL_TABBAR_CONTEXT_MENU, {
        //   commandId: FileNavigatorCommands.REVEAL_IN_NAVIGATOR.id,
        //   label: FileNavigatorCommands.REVEAL_IN_NAVIGATOR.label,
        //   order: '5'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.NAVIGATION, {
        //   commandId: FileNavigatorCommands.OPEN.id,
        //   label: 'Open'
        // });
        // registry.registerSubmenu(NavigatorContextMenu.OPEN_WITH, 'Open With');
        // this.openerService.getOpeners().then(openers => {
        //   for (const opener of openers) {
        //     const openWithCommand = WorkspaceCommands.FILE_OPEN_WITH(opener);
        //     registry.registerMenuAction(NavigatorContextMenu.OPEN_WITH, {
        //       commandId: openWithCommand.id,
        //       label: opener.label,
        //       icon: opener.iconClass
        //     });
        //   }
        // });
        // // registry.registerMenuAction([CONTEXT_MENU_PATH, CUT_MENU_GROUP], {
        // //     commandId: Commands.FILE_CUT
        // // });
        // registry.registerMenuAction(NavigatorContextMenu.CLIPBOARD, {
        //   commandId: CommonCommands.COPY.id,
        //   order: 'a'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.CLIPBOARD, {
        //   commandId: CommonCommands.PASTE.id,
        //   order: 'b'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.CLIPBOARD, {
        //   commandId: CommonCommands.COPY_PATH.id,
        //   order: 'c'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.CLIPBOARD, {
        //   commandId: FileNavigatorCommands.COPY_RELATIVE_FILE_PATH.id,
        //   label: 'Copy Relative Path',
        //   order: 'd'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.CLIPBOARD, {
        //   commandId: FileDownloadCommands.COPY_DOWNLOAD_LINK.id,
        //   order: 'z'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.MODIFICATION, {
        //   commandId: WorkspaceCommands.FILE_RENAME.id
        // });
        // registry.registerMenuAction(NavigatorContextMenu.MODIFICATION, {
        //   commandId: WorkspaceCommands.FILE_DELETE.id
        // });
        // registry.registerMenuAction(NavigatorContextMenu.MODIFICATION, {
        //   commandId: WorkspaceCommands.FILE_DUPLICATE.id
        // });
        // const downloadUploadMenu = [...NAVIGATOR_CONTEXT_MENU, '6_downloadupload'];
        // registry.registerMenuAction(downloadUploadMenu, {
        //   commandId: FileSystemCommands.UPLOAD.id,
        //   order: 'a'
        // });
        // registry.registerMenuAction(downloadUploadMenu, {
        //   commandId: FileDownloadCommands.DOWNLOAD.id,
        //   order: 'b'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.NAVIGATION, {
        //   commandId: WorkspaceCommands.NEW_FILE.id
        // });
        // registry.registerMenuAction(NavigatorContextMenu.NAVIGATION, {
        //   commandId: WorkspaceCommands.NEW_FOLDER.id
        // });
        // registry.registerMenuAction(NavigatorContextMenu.COMPARE, {
        //   commandId: WorkspaceCommands.FILE_COMPARE.id
        // });
        // registry.registerMenuAction(NavigatorContextMenu.MODIFICATION, {
        //   commandId: FileNavigatorCommands.COLLAPSE_ALL.id,
        //   label: 'Collapse All',
        //   order: 'z2'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.COMPARE, {
        //   commandId: NavigatorDiffCommands.COMPARE_FIRST.id,
        //   order: 'z'
        // });
        // registry.registerMenuAction(NavigatorContextMenu.COMPARE, {
        //   commandId: NavigatorDiffCommands.COMPARE_SECOND.id,
        //   order: 'z'
        // });
    };
    FxdkFileNavigatorContribution.prototype.registerKeybindings = function (registry) {
        // super.registerKeybindings(registry);
        return;
        // registry.registerKeybinding({
        //   command: FileNavigatorCommands.REVEAL_IN_NAVIGATOR.id,
        //   keybinding: 'alt+r'
        // });
        // registry.registerKeybinding({
        //   command: WorkspaceCommands.FILE_DELETE.id,
        //   keybinding: isOSX ? 'cmd+backspace' : 'del',
        //   context: NavigatorKeybindingContexts.navigatorActive
        // });
        // registry.registerKeybinding({
        //   command: WorkspaceCommands.FILE_RENAME.id,
        //   keybinding: 'f2',
        //   context: NavigatorKeybindingContexts.navigatorActive
        // });
        // registry.registerKeybinding({
        //   command: FileNavigatorCommands.TOGGLE_HIDDEN_FILES.id,
        //   keybinding: 'ctrlcmd+i',
        //   context: NavigatorKeybindingContexts.navigatorActive
        // });
        // registry.registerKeybinding({
        //   command: FileNavigatorCommands.COPY_RELATIVE_FILE_PATH.id,
        //   keybinding: isWindows ? 'ctrl+k ctrl+shift+c' : 'ctrlcmd+shift+alt+c',
        //   when: '!editorFocus'
        // });
    };
    FxdkFileNavigatorContribution.prototype.registerToolbarItems = function (toolbarRegistry) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    /**
     * Reveals and selects node in the file navigator to which given widget is related.
     * Does nothing if given widget undefined or doesn't have related resource.
     *
     * @param widget widget file resource of which should be revealed and selected
     */
    FxdkFileNavigatorContribution.prototype.selectWidgetFileNode = function (widget) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.onCurrentWidgetChangedHandler = function () {
        // if (this.fileNavigatorPreferences['explorer.autoReveal']) {
        //   this.selectWidgetFileNode(this.shell.currentWidget);
        // }
    };
    /**
     * Collapse file navigator nodes and set focus on first visible node
     * - single root workspace: collapse all nodes except root
     * - multiple root workspace: collapse all nodes, even roots
     */
    FxdkFileNavigatorContribution.prototype.collapseFileNavigatorTree = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    /**
     * force refresh workspace in navigator
     */
    FxdkFileNavigatorContribution.prototype.refreshWorkspace = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    FxdkFileNavigatorContribution.prototype.updateAddRemoveFolderActions = function (registry) {
        this.toDisposeAddRemoveFolderActions.dispose();
        if (this.workspacePreferences['workspace.supportMultiRootWorkspace']) {
            return;
            // this.toDisposeAddRemoveFolderActions.push(registry.registerMenuAction(NavigatorContextMenu.WORKSPACE, {
            //   commandId: FileNavigatorCommands.ADD_ROOT_FOLDER.id,
            //   label: WorkspaceCommands.ADD_FOLDER.label!
            // }));
            // this.toDisposeAddRemoveFolderActions.push(registry.registerMenuAction(NavigatorContextMenu.WORKSPACE, {
            //   commandId: WorkspaceCommands.REMOVE_FOLDER.id
            // }));
        }
    };
    __decorate([
        inversify_1.inject(clipboard_service_1.ClipboardService),
        __metadata("design:type", Object)
    ], FxdkFileNavigatorContribution.prototype, "clipboardService", void 0);
    __decorate([
        inversify_1.inject(common_1.CommandRegistry),
        __metadata("design:type", common_1.CommandRegistry)
    ], FxdkFileNavigatorContribution.prototype, "commandRegistry", void 0);
    __decorate([
        inversify_1.inject(tab_bar_toolbar_1.TabBarToolbarRegistry),
        __metadata("design:type", tab_bar_toolbar_1.TabBarToolbarRegistry)
    ], FxdkFileNavigatorContribution.prototype, "tabbarToolbarRegistry", void 0);
    __decorate([
        inversify_1.inject(navigator_context_key_service_1.NavigatorContextKeyService),
        __metadata("design:type", navigator_context_key_service_1.NavigatorContextKeyService)
    ], FxdkFileNavigatorContribution.prototype, "contextKeyService", void 0);
    __decorate([
        inversify_1.inject(common_1.MenuModelRegistry),
        __metadata("design:type", common_1.MenuModelRegistry)
    ], FxdkFileNavigatorContribution.prototype, "menuRegistry", void 0);
    __decorate([
        inversify_1.inject(navigator_diff_1.NavigatorDiff),
        __metadata("design:type", navigator_diff_1.NavigatorDiff)
    ], FxdkFileNavigatorContribution.prototype, "navigatorDiff", void 0);
    __decorate([
        inversify_1.inject(browser_1.PreferenceService),
        __metadata("design:type", Object)
    ], FxdkFileNavigatorContribution.prototype, "preferenceService", void 0);
    __decorate([
        inversify_1.inject(selection_service_1.SelectionService),
        __metadata("design:type", selection_service_1.SelectionService)
    ], FxdkFileNavigatorContribution.prototype, "selectionService", void 0);
    __decorate([
        inversify_1.inject(browser_2.WorkspaceCommandContribution),
        __metadata("design:type", browser_2.WorkspaceCommandContribution)
    ], FxdkFileNavigatorContribution.prototype, "workspaceCommandContribution", void 0);
    __decorate([
        inversify_1.postConstruct(),
        __metadata("design:type", Function),
        __metadata("design:paramtypes", []),
        __metadata("design:returntype", Promise)
    ], FxdkFileNavigatorContribution.prototype, "init", null);
    FxdkFileNavigatorContribution = __decorate([
        inversify_1.injectable(),
        __param(0, inversify_1.inject(navigator_preferences_1.FileNavigatorPreferences)),
        __param(1, inversify_1.inject(browser_1.OpenerService)),
        __param(2, inversify_1.inject(navigator_filter_1.FileNavigatorFilter)),
        __param(3, inversify_1.inject(browser_2.WorkspaceService)),
        __param(4, inversify_1.inject(browser_2.WorkspacePreferences)),
        __metadata("design:paramtypes", [Object, Object, navigator_filter_1.FileNavigatorFilter,
            browser_2.WorkspaceService, Object])
    ], FxdkFileNavigatorContribution);
    return FxdkFileNavigatorContribution;
}());
exports.FxdkFileNavigatorContribution = FxdkFileNavigatorContribution;
exports.rebindNavigator = function (bind, rebind) {
    bind(FxdkFileNavigatorContribution).toSelf().inSingletonScope();
    rebind(navigator_contribution_1.FileNavigatorContribution).toService(FxdkFileNavigatorContribution);
};
//# sourceMappingURL=rebindNavigator.js.map