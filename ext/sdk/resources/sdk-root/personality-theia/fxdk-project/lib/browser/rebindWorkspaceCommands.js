"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
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
var __values = (this && this.__values) || function(o) {
    var s = typeof Symbol === "function" && Symbol.iterator, m = s && o[s], i = 0;
    if (m) return m.call(o);
    if (o && typeof o.length === "number") return {
        next: function () {
            if (o && i >= o.length) o = void 0;
            return { value: o && o[i++], done: !o };
        }
    };
    throw new TypeError(s ? "Object is not iterable." : "Symbol.iterator is not defined.");
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
exports.rebindWorkspaceCommands = exports.WorkspaceRootUriAwareCommandHandler = exports.FxdkWorkspaceCommandContribution = exports.FxdkEditMenuContribution = exports.FxdkFileMenuContribution = exports.WorkspaceCommands = void 0;
var inversify_1 = require("inversify");
var selection_service_1 = require("@theia/core/lib/common/selection-service");
var browser_1 = require("@theia/filesystem/lib/browser");
var dialogs_1 = require("@theia/core/lib/browser/dialogs");
var browser_2 = require("@theia/core/lib/browser");
var uri_command_handler_1 = require("@theia/core/lib/common/uri-command-handler");
var browser_3 = require("@theia/workspace/lib/browser");
var message_service_1 = require("@theia/core/lib/common/message-service");
var browser_4 = require("@theia/workspace/lib/browser");
var workspace_delete_handler_1 = require("@theia/workspace/lib/browser/workspace-delete-handler");
var workspace_duplicate_handler_1 = require("@theia/workspace/lib/browser/workspace-duplicate-handler");
var workspace_compare_handler_1 = require("@theia/workspace/lib/browser/workspace-compare-handler");
var common_1 = require("@theia/core/lib/common");
var file_service_1 = require("@theia/filesystem/lib/browser/file-service");
var validFilename = require('valid-filename');
var WorkspaceCommands;
(function (WorkspaceCommands) {
    var WORKSPACE_CATEGORY = 'Workspace';
    var FILE_CATEGORY = 'File';
    // On Linux and Windows, both files and folders cannot be opened at the same time in electron.
    // `OPEN_FILE` and `OPEN_FOLDER` must be available only on Linux and Windows in electron.
    // `OPEN` must *not* be available on Windows and Linux in electron.
    // VS Code does the same. See: https://github.com/eclipse-theia/theia/pull/3202#issuecomment-430585357
    WorkspaceCommands.OPEN = {
        id: 'workspace:open',
        category: FILE_CATEGORY,
        label: 'Open...',
        dialogLabel: 'Open'
    };
    // No `label`. Otherwise, it shows up in the `Command Palette`.
    WorkspaceCommands.OPEN_FILE = {
        id: 'workspace:openFile',
        category: FILE_CATEGORY,
        dialogLabel: 'Open File'
    };
    WorkspaceCommands.OPEN_FOLDER = {
        id: 'workspace:openFolder',
        dialogLabel: 'Open Folder' // No `label`. Otherwise, it shows up in the `Command Palette`.
    };
    WorkspaceCommands.OPEN_WORKSPACE = {
        id: 'workspace:openWorkspace',
        category: FILE_CATEGORY,
        label: 'Open Project...',
        dialogLabel: 'Open Project'
    };
    WorkspaceCommands.OPEN_RECENT_WORKSPACE = {
        id: 'workspace:openRecent',
        category: FILE_CATEGORY,
        label: 'Open Recent Project...'
    };
    WorkspaceCommands.CLOSE = {
        id: 'workspace:close',
        category: WORKSPACE_CATEGORY,
        label: 'Close Project'
    };
    WorkspaceCommands.NEW_FILE = {
        id: 'file.newFile',
        category: FILE_CATEGORY,
        label: 'New File'
    };
    WorkspaceCommands.NEW_FOLDER = {
        id: 'file.newFolder',
        category: FILE_CATEGORY,
        label: 'New Folder'
    };
    WorkspaceCommands.FILE_OPEN_WITH = function (opener) { return ({
        id: "file.openWith." + opener.id
    }); };
    WorkspaceCommands.FILE_RENAME = {
        id: 'file.rename',
        category: FILE_CATEGORY,
        label: 'Rename'
    };
    WorkspaceCommands.FILE_DELETE = {
        id: 'file.delete',
        category: FILE_CATEGORY,
        label: 'Delete'
    };
    WorkspaceCommands.FILE_DUPLICATE = {
        id: 'file.duplicate',
        category: FILE_CATEGORY,
        label: 'Duplicate'
    };
    WorkspaceCommands.FILE_COMPARE = {
        id: 'file.compare',
        category: FILE_CATEGORY,
        label: 'Compare with Each Other'
    };
    WorkspaceCommands.ADD_FOLDER = {
        id: 'workspace:addFolder',
        category: WORKSPACE_CATEGORY,
        label: 'Add Folder to Project...'
    };
    WorkspaceCommands.REMOVE_FOLDER = {
        id: 'workspace:removeFolder',
        category: WORKSPACE_CATEGORY,
        label: 'Remove Folder from Project'
    };
    WorkspaceCommands.SAVE_WORKSPACE_AS = {
        id: 'workspace:saveAs',
        category: WORKSPACE_CATEGORY,
        label: 'Save Project As...'
    };
    WorkspaceCommands.SAVE_AS = {
        id: 'file.saveAs',
        category: 'File',
        label: 'Save As...',
    };
})(WorkspaceCommands = exports.WorkspaceCommands || (exports.WorkspaceCommands = {}));
var FxdkFileMenuContribution = /** @class */ (function () {
    function FxdkFileMenuContribution() {
    }
    FxdkFileMenuContribution.prototype.registerMenus = function (registry) {
    };
    FxdkFileMenuContribution = __decorate([
        inversify_1.injectable()
    ], FxdkFileMenuContribution);
    return FxdkFileMenuContribution;
}());
exports.FxdkFileMenuContribution = FxdkFileMenuContribution;
var FxdkEditMenuContribution = /** @class */ (function () {
    function FxdkEditMenuContribution() {
    }
    FxdkEditMenuContribution.prototype.registerMenus = function (registry) {
        // registry.registerMenuAction(CommonMenus.EDIT_CLIPBOARD, {
        //   commandId: FileDownloadCommands.COPY_DOWNLOAD_LINK.id,
        //   order: '9999'
        // });
    };
    FxdkEditMenuContribution = __decorate([
        inversify_1.injectable()
    ], FxdkEditMenuContribution);
    return FxdkEditMenuContribution;
}());
exports.FxdkEditMenuContribution = FxdkEditMenuContribution;
var FxdkWorkspaceCommandContribution = /** @class */ (function () {
    function FxdkWorkspaceCommandContribution() {
        this.onDidCreateNewFileEmitter = new common_1.Emitter();
        this.onDidCreateNewFolderEmitter = new common_1.Emitter();
    }
    Object.defineProperty(FxdkWorkspaceCommandContribution.prototype, "onDidCreateNewFile", {
        get: function () {
            return this.onDidCreateNewFileEmitter.event;
        },
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(FxdkWorkspaceCommandContribution.prototype, "onDidCreateNewFolder", {
        get: function () {
            return this.onDidCreateNewFolderEmitter.event;
        },
        enumerable: false,
        configurable: true
    });
    FxdkWorkspaceCommandContribution.prototype.fireCreateNewFile = function (uri) {
        this.onDidCreateNewFileEmitter.fire(uri);
    };
    FxdkWorkspaceCommandContribution.prototype.fireCreateNewFolder = function (uri) {
        this.onDidCreateNewFolderEmitter.fire(uri);
    };
    FxdkWorkspaceCommandContribution.prototype.registerCommands = function (registry) {
        // this.openerService.getOpeners().then(openers => {
        //   for (const opener of openers) {
        //     const openWithCommand = WorkspaceCommands.FILE_OPEN_WITH(opener);
        //     registry.registerCommand(openWithCommand, this.newUriAwareCommandHandler({
        //       execute: uri => opener.open(uri),
        //       isEnabled: uri => opener.canHandle(uri) > 0,
        //       isVisible: uri => opener.canHandle(uri) > 0 && this.areMultipleOpenHandlersPresent(openers, uri)
        //     }));
        //   }
        // });
        // registry.registerCommand(WorkspaceCommands.NEW_FILE, this.newWorkspaceRootUriAwareCommandHandler({
        //   execute: uri => this.getDirectory(uri).then(parent => {
        //     if (parent) {
        //       const parentUri = parent.resource;
        //       const { fileName, fileExtension } = this.getDefaultFileConfig();
        //       const vacantChildUri = FileSystemUtils.generateUniqueResourceURI(parentUri, parent, fileName, fileExtension);
        //       const dialog = new WorkspaceInputDialog({
        //         title: 'New File',
        //         parentUri: parentUri,
        //         initialValue: vacantChildUri.path.base,
        //         validate: name => this.validateFileName(name, parent, true)
        //       }, this.labelProvider);
        //       dialog.open().then(async name => {
        //         if (name) {
        //           const fileUri = parentUri.resolve(name);
        //           await this.fileService.create(fileUri);
        //           this.fireCreateNewFile({ parent: parentUri, uri: fileUri });
        //           open(this.openerService, fileUri);
        //         }
        //       });
        //     }
        //   })
        // }));
        // registry.registerCommand(WorkspaceCommands.NEW_FOLDER, this.newWorkspaceRootUriAwareCommandHandler({
        //   execute: uri => this.getDirectory(uri).then(parent => {
        //     if (parent) {
        //       const parentUri = parent.resource;
        //       const vacantChildUri = FileSystemUtils.generateUniqueResourceURI(parentUri, parent, 'Untitled');
        //       const dialog = new WorkspaceInputDialog({
        //         title: 'New Folder',
        //         parentUri: parentUri,
        //         initialValue: vacantChildUri.path.base,
        //         validate: name => this.validateFileName(name, parent, true)
        //       }, this.labelProvider);
        //       dialog.open().then(async name => {
        //         if (name) {
        //           const folderUri = parentUri.resolve(name);
        //           await this.fileService.createFolder(folderUri);
        //           this.fireCreateNewFile({ parent: parentUri, uri: folderUri });
        //         }
        //       });
        //     }
        //   })
        // }));
        // registry.registerCommand(WorkspaceCommands.FILE_RENAME, this.newMultiUriAwareCommandHandler({
        //   isEnabled: uris => uris.some(uri => !this.isWorkspaceRoot(uri)) && uris.length === 1,
        //   isVisible: uris => uris.some(uri => !this.isWorkspaceRoot(uri)) && uris.length === 1,
        //   execute: (uris): void => {
        //     uris.forEach(async uri => {
        //       const parent = await this.getParent(uri);
        //       if (parent) {
        //         const initialValue = uri.path.base;
        //         const stat = await this.fileService.resolve(uri);
        //         const fileType = stat.isDirectory ? 'Directory' : 'File';
        //         const titleStr = `Rename ${fileType}`;
        //         const dialog = new SingleTextInputDialog({
        //           title: titleStr,
        //           initialValue,
        //           initialSelectionRange: {
        //             start: 0,
        //             end: uri.path.name.length
        //           },
        //           validate: (name, mode) => {
        //             if (initialValue === name && mode === 'preview') {
        //               return false;
        //             }
        //             return this.validateFileName(name, parent, false);
        //           }
        //         });
        //         const fileName = await dialog.open();
        //         if (fileName) {
        //           const oldUri = uri;
        //           const newUri = uri.parent.resolve(fileName);
        //           this.fileService.move(oldUri, newUri);
        //         }
        //       }
        //     });
        //   }
        // }));
        // registry.registerCommand(WorkspaceCommands.FILE_DUPLICATE, this.newMultiUriAwareCommandHandler(this.duplicateHandler));
        // registry.registerCommand(WorkspaceCommands.FILE_DELETE, this.newMultiUriAwareCommandHandler(this.deleteHandler));
        // registry.registerCommand(WorkspaceCommands.FILE_COMPARE, this.newMultiUriAwareCommandHandler(this.compareHandler));
    };
    FxdkWorkspaceCommandContribution.prototype.newUriAwareCommandHandler = function (handler) {
        return new uri_command_handler_1.UriAwareCommandHandler(this.selectionService, handler);
    };
    FxdkWorkspaceCommandContribution.prototype.newMultiUriAwareCommandHandler = function (handler) {
        return new uri_command_handler_1.UriAwareCommandHandler(this.selectionService, handler, { multi: true });
    };
    FxdkWorkspaceCommandContribution.prototype.newWorkspaceRootUriAwareCommandHandler = function (handler) {
        return new WorkspaceRootUriAwareCommandHandler(this.workspaceService, this.selectionService, handler);
    };
    /**
     * Returns an error message if the file name is invalid. Otherwise, an empty string.
     *
     * @param name the simple file name of the file to validate.
     * @param parent the parent directory's file stat.
     * @param recursive allow file or folder creation using recursive path
     */
    FxdkWorkspaceCommandContribution.prototype.validateFileName = function (name, parent, recursive) {
        if (recursive === void 0) { recursive = false; }
        return __awaiter(this, void 0, void 0, function () {
            var childUri, exists;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!name) {
                            return [2 /*return*/, ''];
                        }
                        // do not allow recursive rename
                        if (!recursive && !validFilename(name)) {
                            return [2 /*return*/, 'Invalid file or folder name'];
                        }
                        if (name.startsWith('/')) {
                            return [2 /*return*/, 'Absolute paths or names that starts with / are not allowed'];
                        }
                        else if (name.startsWith(' ') || name.endsWith(' ')) {
                            return [2 /*return*/, 'Names with leading or trailing whitespaces are not allowed'];
                        }
                        // check and validate each sub-paths
                        if (name.split(/[\\/]/).some(function (file) { return !file || !validFilename(file) || /^\s+$/.test(file); })) {
                            return [2 /*return*/, "The name \"" + this.trimFileName(name) + "\" is not a valid file or folder name."];
                        }
                        childUri = parent.resource.resolve(name);
                        return [4 /*yield*/, this.fileService.exists(childUri)];
                    case 1:
                        exists = _a.sent();
                        if (exists) {
                            return [2 /*return*/, "A file or folder \"" + this.trimFileName(name) + "\" already exists at this location."];
                        }
                        return [2 /*return*/, ''];
                }
            });
        });
    };
    FxdkWorkspaceCommandContribution.prototype.trimFileName = function (name) {
        if (name && name.length > 30) {
            return name.substr(0, 30) + "...";
        }
        return name;
    };
    FxdkWorkspaceCommandContribution.prototype.getDirectory = function (candidate) {
        return __awaiter(this, void 0, void 0, function () {
            var stat, _a;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0:
                        _b.trys.push([0, 2, , 3]);
                        return [4 /*yield*/, this.fileService.resolve(candidate)];
                    case 1:
                        stat = _b.sent();
                        return [3 /*break*/, 3];
                    case 2:
                        _a = _b.sent();
                        return [3 /*break*/, 3];
                    case 3:
                        if (stat && stat.isDirectory) {
                            return [2 /*return*/, stat];
                        }
                        return [2 /*return*/, this.getParent(candidate)];
                }
            });
        });
    };
    FxdkWorkspaceCommandContribution.prototype.getParent = function (candidate) {
        return __awaiter(this, void 0, void 0, function () {
            var _a;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0:
                        _b.trys.push([0, 2, , 3]);
                        return [4 /*yield*/, this.fileService.resolve(candidate.parent)];
                    case 1: return [2 /*return*/, _b.sent()];
                    case 2:
                        _a = _b.sent();
                        return [2 /*return*/, undefined];
                    case 3: return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceCommandContribution.prototype.addFolderToWorkspace = function (uri) {
        return __awaiter(this, void 0, void 0, function () {
            var stat, _a;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0:
                        if (!uri) return [3 /*break*/, 6];
                        _b.label = 1;
                    case 1:
                        _b.trys.push([1, 5, , 6]);
                        return [4 /*yield*/, this.fileService.resolve(uri)];
                    case 2:
                        stat = _b.sent();
                        if (!stat.isDirectory) return [3 /*break*/, 4];
                        return [4 /*yield*/, this.workspaceService.addRoot(uri)];
                    case 3:
                        _b.sent();
                        _b.label = 4;
                    case 4: return [3 /*break*/, 6];
                    case 5:
                        _a = _b.sent();
                        return [3 /*break*/, 6];
                    case 6: return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceCommandContribution.prototype.areWorkspaceRoots = function (uris) {
        return this.workspaceService.areWorkspaceRoots(uris);
    };
    FxdkWorkspaceCommandContribution.prototype.isWorkspaceRoot = function (uri) {
        var rootUris = new Set(this.workspaceService.tryGetRoots().map(function (root) { return root.resource.toString(); }));
        return rootUris.has(uri.toString());
    };
    FxdkWorkspaceCommandContribution.prototype.getDefaultFileConfig = function () {
        return {
            fileName: 'Untitled',
            fileExtension: '.txt'
        };
    };
    /**
     * Removes the list of folders from the workspace upon confirmation from the user.
     * @param uris the list of folder uris to remove.
     */
    FxdkWorkspaceCommandContribution.prototype.removeFolderFromWorkspace = function (uris) {
        return __awaiter(this, void 0, void 0, function () {
            var roots, toRemove, messageContainer, list_1, dialog;
            var _this = this;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        roots = new Set(this.workspaceService.tryGetRoots().map(function (root) { return root.resource.toString(); }));
                        toRemove = uris.filter(function (uri) { return roots.has(uri.toString()); });
                        if (!(toRemove.length > 0)) return [3 /*break*/, 3];
                        messageContainer = document.createElement('div');
                        messageContainer.textContent = "Are you sure you want to remove the following folder" + (toRemove.length > 1 ? 's' : '') + " from the project?";
                        messageContainer.title = 'Note: Nothing will be erased from disk';
                        list_1 = document.createElement('div');
                        list_1.classList.add('theia-dialog-node');
                        toRemove.forEach(function (uri) {
                            var listItem = document.createElement('div');
                            listItem.classList.add('theia-dialog-node-content');
                            var folderIcon = document.createElement('span');
                            folderIcon.classList.add('codicon', 'codicon-root-folder', 'theia-dialog-icon');
                            listItem.appendChild(folderIcon);
                            listItem.title = _this.labelProvider.getLongName(uri);
                            var listContent = document.createElement('span');
                            listContent.classList.add('theia-dialog-node-segment');
                            listContent.appendChild(document.createTextNode(_this.labelProvider.getName(uri)));
                            listItem.appendChild(listContent);
                            list_1.appendChild(listItem);
                        });
                        messageContainer.appendChild(list_1);
                        dialog = new dialogs_1.ConfirmDialog({
                            title: 'Remove Folder from Project',
                            msg: messageContainer
                        });
                        return [4 /*yield*/, dialog.open()];
                    case 1:
                        if (!_a.sent()) return [3 /*break*/, 3];
                        return [4 /*yield*/, this.workspaceService.removeRoots(toRemove)];
                    case 2:
                        _a.sent();
                        _a.label = 3;
                    case 3: return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceCommandContribution.prototype.areMultipleOpenHandlersPresent = function (openers, uri) {
        var e_1, _a;
        var count = 0;
        try {
            for (var openers_1 = __values(openers), openers_1_1 = openers_1.next(); !openers_1_1.done; openers_1_1 = openers_1.next()) {
                var opener_1 = openers_1_1.value;
                if (opener_1.canHandle(uri) > 0) {
                    count++;
                }
                if (count > 1) {
                    return true;
                }
            }
        }
        catch (e_1_1) { e_1 = { error: e_1_1 }; }
        finally {
            try {
                if (openers_1_1 && !openers_1_1.done && (_a = openers_1.return)) _a.call(openers_1);
            }
            finally { if (e_1) throw e_1.error; }
        }
        return false;
    };
    __decorate([
        inversify_1.inject(browser_2.LabelProvider),
        __metadata("design:type", browser_2.LabelProvider)
    ], FxdkWorkspaceCommandContribution.prototype, "labelProvider", void 0);
    __decorate([
        inversify_1.inject(file_service_1.FileService),
        __metadata("design:type", file_service_1.FileService)
    ], FxdkWorkspaceCommandContribution.prototype, "fileService", void 0);
    __decorate([
        inversify_1.inject(browser_3.WorkspaceService),
        __metadata("design:type", browser_3.WorkspaceService)
    ], FxdkWorkspaceCommandContribution.prototype, "workspaceService", void 0);
    __decorate([
        inversify_1.inject(selection_service_1.SelectionService),
        __metadata("design:type", selection_service_1.SelectionService)
    ], FxdkWorkspaceCommandContribution.prototype, "selectionService", void 0);
    __decorate([
        inversify_1.inject(browser_2.OpenerService),
        __metadata("design:type", Object)
    ], FxdkWorkspaceCommandContribution.prototype, "openerService", void 0);
    __decorate([
        inversify_1.inject(browser_2.FrontendApplication),
        __metadata("design:type", browser_2.FrontendApplication)
    ], FxdkWorkspaceCommandContribution.prototype, "app", void 0);
    __decorate([
        inversify_1.inject(message_service_1.MessageService),
        __metadata("design:type", message_service_1.MessageService)
    ], FxdkWorkspaceCommandContribution.prototype, "messageService", void 0);
    __decorate([
        inversify_1.inject(browser_4.WorkspacePreferences),
        __metadata("design:type", Object)
    ], FxdkWorkspaceCommandContribution.prototype, "preferences", void 0);
    __decorate([
        inversify_1.inject(browser_1.FileDialogService),
        __metadata("design:type", Object)
    ], FxdkWorkspaceCommandContribution.prototype, "fileDialogService", void 0);
    __decorate([
        inversify_1.inject(workspace_delete_handler_1.WorkspaceDeleteHandler),
        __metadata("design:type", workspace_delete_handler_1.WorkspaceDeleteHandler)
    ], FxdkWorkspaceCommandContribution.prototype, "deleteHandler", void 0);
    __decorate([
        inversify_1.inject(workspace_duplicate_handler_1.WorkspaceDuplicateHandler),
        __metadata("design:type", workspace_duplicate_handler_1.WorkspaceDuplicateHandler)
    ], FxdkWorkspaceCommandContribution.prototype, "duplicateHandler", void 0);
    __decorate([
        inversify_1.inject(workspace_compare_handler_1.WorkspaceCompareHandler),
        __metadata("design:type", workspace_compare_handler_1.WorkspaceCompareHandler)
    ], FxdkWorkspaceCommandContribution.prototype, "compareHandler", void 0);
    FxdkWorkspaceCommandContribution = __decorate([
        inversify_1.injectable()
    ], FxdkWorkspaceCommandContribution);
    return FxdkWorkspaceCommandContribution;
}());
exports.FxdkWorkspaceCommandContribution = FxdkWorkspaceCommandContribution;
var WorkspaceRootUriAwareCommandHandler = /** @class */ (function (_super) {
    __extends(WorkspaceRootUriAwareCommandHandler, _super);
    function WorkspaceRootUriAwareCommandHandler(workspaceService, selectionService, handler) {
        var _this = _super.call(this, selectionService, handler) || this;
        _this.workspaceService = workspaceService;
        _this.selectionService = selectionService;
        _this.handler = handler;
        return _this;
    }
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    WorkspaceRootUriAwareCommandHandler.prototype.isEnabled = function () {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        return _super.prototype.isEnabled.apply(this, __spread(args)) && !!this.workspaceService.tryGetRoots().length;
    };
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    WorkspaceRootUriAwareCommandHandler.prototype.isVisible = function () {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        return _super.prototype.isVisible.apply(this, __spread(args)) && !!this.workspaceService.tryGetRoots().length;
    };
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    WorkspaceRootUriAwareCommandHandler.prototype.getUri = function () {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        var uri = _super.prototype.getUri.apply(this, __spread(args));
        // Return the `uri` immediately if the resource exists in any of the workspace roots and is of `file` scheme.
        if (uri && uri.scheme === 'file' && this.workspaceService.getWorkspaceRootUri(uri)) {
            return uri;
        }
        // Return the first root if available.
        if (!!this.workspaceService.tryGetRoots().length) {
            return this.workspaceService.tryGetRoots()[0].resource;
        }
    };
    return WorkspaceRootUriAwareCommandHandler;
}(uri_command_handler_1.UriAwareCommandHandler));
exports.WorkspaceRootUriAwareCommandHandler = WorkspaceRootUriAwareCommandHandler;
function rebindWorkspaceCommands(bind, rebind) {
    bind(FxdkWorkspaceCommandContribution).toSelf().inSingletonScope();
    rebind(browser_3.WorkspaceCommandContribution).toService(FxdkWorkspaceCommandContribution);
    bind(FxdkFileMenuContribution).toSelf().inSingletonScope();
    rebind(browser_3.FileMenuContribution).toService(FxdkFileMenuContribution);
    bind(FxdkEditMenuContribution).toSelf().inSingletonScope();
    rebind(browser_3.EditMenuContribution).toService(FxdkEditMenuContribution);
}
exports.rebindWorkspaceCommands = rebindWorkspaceCommands;
//# sourceMappingURL=rebindWorkspaceCommands.js.map