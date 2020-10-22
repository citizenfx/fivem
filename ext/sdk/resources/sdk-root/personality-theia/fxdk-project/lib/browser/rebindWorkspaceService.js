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
exports.rebindWorkspaceService = exports.WorkspaceData = exports.FxdkWorkspaceService = void 0;
var inversify_1 = require("inversify");
var browser_1 = require("@theia/workspace/lib/browser");
var uri_1 = require("@theia/core/lib/common/uri");
var common_1 = require("@theia/workspace/lib/common");
var window_service_1 = require("@theia/core/lib/browser/window/window-service");
var browser_2 = require("@theia/core/lib/browser");
var promise_util_1 = require("@theia/core/lib/common/promise-util");
var env_variables_1 = require("@theia/core/lib/common/env-variables");
var core_1 = require("@theia/core");
var workspace_preferences_1 = require("@theia/workspace/lib/browser/workspace-preferences");
var Ajv = require("ajv");
var frontend_application_config_provider_1 = require("@theia/core/lib/browser/frontend-application-config-provider");
var files_1 = require("@theia/filesystem/lib/common/files");
var file_service_1 = require("@theia/filesystem/lib/browser/file-service");
var browser_3 = require("@theia/filesystem/lib/browser");
var FxdkWorkspaceService = /** @class */ (function () {
    function FxdkWorkspaceService() {
        // FxDK
        this._folders = new Set();
        this._projectName = 'none';
        this._projectPath = 'none';
        this._roots = [];
        this.deferredRoots = new promise_util_1.Deferred();
        this.onWorkspaceChangeEmitter = new core_1.Emitter();
        this.onWorkspaceLocationChangedEmitter = new core_1.Emitter();
        this.rootWatchers = new Map();
    }
    FxdkWorkspaceService.prototype.init = function () {
        return __awaiter(this, void 0, void 0, function () {
            var _this = this;
            return __generator(this, function (_a) {
                this.applicationName = frontend_application_config_provider_1.FrontendApplicationConfigProvider.get().applicationName;
                this.fsPreferences.onPreferenceChanged(function (e) {
                    if (e.preferenceName === 'files.watcherExclude') {
                        _this.refreshRootWatchers();
                    }
                });
                return [2 /*return*/];
            });
        });
    };
    Object.defineProperty(FxdkWorkspaceService.prototype, "roots", {
        get: function () {
            return this.deferredRoots.promise;
        },
        enumerable: false,
        configurable: true
    });
    FxdkWorkspaceService.prototype.tryGetRoots = function () {
        return this._roots;
    };
    Object.defineProperty(FxdkWorkspaceService.prototype, "workspace", {
        get: function () {
            var _this = this;
            if (!this._projectUri) {
                return;
            }
            return {
                isDirectory: false,
                resource: this._projectUri || {
                    path: {
                        isAbsolute: false,
                        isRoot: false,
                        name: this._projectName,
                    },
                    toString: function () {
                        return _this._projectPath;
                    },
                },
            };
        },
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(FxdkWorkspaceService.prototype, "onWorkspaceChanged", {
        get: function () {
            return this.onWorkspaceChangeEmitter.event;
        },
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(FxdkWorkspaceService.prototype, "onWorkspaceLocationChanged", {
        get: function () {
            return this.onWorkspaceLocationChangedEmitter.event;
        },
        enumerable: false,
        configurable: true
    });
    FxdkWorkspaceService.prototype.setProject = function (_a) {
        var name = _a.name, path = _a.path, folders = _a.folders;
        this._projectName = name;
        this._projectPath = path;
        this._projectUri = new uri_1.default('file:///' + this._projectPath + '/.fxdk/theia-settings.json');
        this._folders = new Set(folders);
        console.log('Opened project in theia', name, path, folders);
        this.onWorkspaceLocationChangedEmitter.fire(this.workspace);
        this.updateWorkspace();
    };
    FxdkWorkspaceService.prototype.addFolder = function (dir) {
        if (!this._folders.has(dir)) {
            this._folders.add(dir);
            this.updateWorkspace();
        }
    };
    FxdkWorkspaceService.prototype.addFolders = function (dirs) {
        var _this = this;
        dirs
            .filter(function (dir) { return !_this._folders.has(dir); })
            .forEach(function (dir) { return _this._folders.add(dir); });
        this.updateWorkspace();
    };
    FxdkWorkspaceService.prototype.setFolders = function (dirs) {
        this._folders = new Set(dirs);
        this.updateWorkspace();
    };
    FxdkWorkspaceService.prototype.removeFolder = function (dir) {
        this._folders.delete(dir);
        this.updateWorkspace();
    };
    FxdkWorkspaceService.prototype.clearFolders = function () {
        this._folders = new Set();
        this.updateWorkspace();
    };
    FxdkWorkspaceService.prototype.updateWorkspace = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.updateRoots()];
                    case 1:
                        _a.sent();
                        this.watchRoots();
                        return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceService.prototype.updateRoots = function () {
        return __awaiter(this, void 0, void 0, function () {
            var newRoots, rootsChanged, _loop_1, this_1, newRoots_1, newRoots_1_1, newRoot, state_1;
            var e_1, _a;
            return __generator(this, function (_b) {
                switch (_b.label) {
                    case 0: return [4 /*yield*/, this.computeRoots()];
                    case 1:
                        newRoots = _b.sent();
                        rootsChanged = false;
                        if (newRoots.length !== this._roots.length || newRoots.length === 0) {
                            rootsChanged = true;
                        }
                        else {
                            _loop_1 = function (newRoot) {
                                if (!this_1._roots.some(function (r) { return r.resource.toString() === newRoot.resource.toString(); })) {
                                    rootsChanged = true;
                                    return "break";
                                }
                            };
                            this_1 = this;
                            try {
                                for (newRoots_1 = __values(newRoots), newRoots_1_1 = newRoots_1.next(); !newRoots_1_1.done; newRoots_1_1 = newRoots_1.next()) {
                                    newRoot = newRoots_1_1.value;
                                    state_1 = _loop_1(newRoot);
                                    if (state_1 === "break")
                                        break;
                                }
                            }
                            catch (e_1_1) { e_1 = { error: e_1_1 }; }
                            finally {
                                try {
                                    if (newRoots_1_1 && !newRoots_1_1.done && (_a = newRoots_1.return)) _a.call(newRoots_1);
                                }
                                finally { if (e_1) throw e_1.error; }
                            }
                        }
                        if (rootsChanged) {
                            this._roots = newRoots;
                            this.deferredRoots.resolve(this._roots); // in order to resolve first
                            this.deferredRoots = new promise_util_1.Deferred();
                            this.deferredRoots.resolve(this._roots);
                            this.onWorkspaceChangeEmitter.fire(this._roots);
                        }
                        return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceService.prototype.computeRoots = function () {
        return __awaiter(this, void 0, void 0, function () {
            var roots, _a, _b, path, valid, e_2_1;
            var e_2, _c;
            return __generator(this, function (_d) {
                switch (_d.label) {
                    case 0:
                        roots = [];
                        _d.label = 1;
                    case 1:
                        _d.trys.push([1, 6, 7, 8]);
                        _a = __values(this._folders), _b = _a.next();
                        _d.label = 2;
                    case 2:
                        if (!!_b.done) return [3 /*break*/, 5];
                        path = _b.value;
                        return [4 /*yield*/, this.toValidRoot(path)];
                    case 3:
                        valid = _d.sent();
                        if (valid) {
                            roots.push(valid);
                        }
                        else {
                            roots.push(files_1.FileStat.dir(path));
                        }
                        _d.label = 4;
                    case 4:
                        _b = _a.next();
                        return [3 /*break*/, 2];
                    case 5: return [3 /*break*/, 8];
                    case 6:
                        e_2_1 = _d.sent();
                        e_2 = { error: e_2_1 };
                        return [3 /*break*/, 8];
                    case 7:
                        try {
                            if (_b && !_b.done && (_c = _a.return)) _c.call(_a);
                        }
                        finally { if (e_2) throw e_2.error; }
                        return [7 /*endfinally*/];
                    case 8: return [2 /*return*/, roots];
                }
            });
        });
    };
    FxdkWorkspaceService.prototype.getWorkspaceDataFromFile = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, {
                        folders: __spread(this._folders.values()).map(function (path) { return ({ path: path }); }),
                    }];
            });
        });
    };
    /**
     * on unload, we set our workspace root as the last recently used on the backend.
     */
    FxdkWorkspaceService.prototype.onStop = function () {
        return;
    };
    FxdkWorkspaceService.prototype.recentWorkspaces = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, []];
            });
        });
    };
    Object.defineProperty(FxdkWorkspaceService.prototype, "opened", {
        /**
         * Returns `true` if theia has an opened workspace or folder
         * @returns {boolean}
         */
        get: function () {
            return true;
        },
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(FxdkWorkspaceService.prototype, "isMultiRootWorkspaceOpened", {
        /**
         * Returns `true` if a multiple-root workspace is currently open.
         * @returns {boolean}
         */
        get: function () {
            return !!this.workspace && true;
        },
        enumerable: false,
        configurable: true
    });
    Object.defineProperty(FxdkWorkspaceService.prototype, "isMultiRootWorkspaceEnabled", {
        /**
         * Returns `true` if there is an opened workspace, and multi root workspace support is enabled.
         * @returns {boolean}
         */
        get: function () {
            return this.opened && true;
        },
        enumerable: false,
        configurable: true
    });
    /**
     * Opens directory, or recreates a workspace from the file that `uri` points to.
     */
    FxdkWorkspaceService.prototype.open = function (uri, options) {
        return;
    };
    /**
     * Clears current workspace root.
     */
    FxdkWorkspaceService.prototype.close = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                this._roots.length = 0;
                return [2 /*return*/];
            });
        });
    };
    /**
     * returns a FileStat if the argument URI points to an existing directory. Otherwise, `undefined`.
     */
    FxdkWorkspaceService.prototype.toValidRoot = function (uri) {
        return __awaiter(this, void 0, void 0, function () {
            var fileStat;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, this.toFileStat(uri)];
                    case 1:
                        fileStat = _a.sent();
                        if (fileStat && fileStat.isDirectory) {
                            return [2 /*return*/, fileStat];
                        }
                        return [2 /*return*/, undefined];
                }
            });
        });
    };
    /**
     * returns a FileStat if the argument URI points to a file or directory. Otherwise, `undefined`.
     */
    FxdkWorkspaceService.prototype.toFileStat = function (uri) {
        return __awaiter(this, void 0, void 0, function () {
            var uriStr, normalizedUri, error_1;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!uri) {
                            return [2 /*return*/, undefined];
                        }
                        uriStr = uri.toString();
                        _a.label = 1;
                    case 1:
                        _a.trys.push([1, 3, , 4]);
                        if (uriStr.endsWith('/')) {
                            uriStr = uriStr.slice(0, -1);
                        }
                        normalizedUri = new uri_1.default(uriStr).normalizePath();
                        return [4 /*yield*/, this.fileService.resolve(normalizedUri)];
                    case 2: return [2 /*return*/, _a.sent()];
                    case 3:
                        error_1 = _a.sent();
                        return [2 /*return*/, undefined];
                    case 4: return [2 /*return*/];
                }
            });
        });
    };
    /**
     * Return true if one of the paths in paths array is present in the workspace
     * NOTE: You should always explicitly use `/` as the separator between the path segments.
     */
    FxdkWorkspaceService.prototype.containsSome = function (paths) {
        return __awaiter(this, void 0, void 0, function () {
            var _a, _b, root, uri, paths_1, paths_1_1, path, fileUri, exists, e_3_1, e_4_1;
            var e_4, _c, e_3, _d;
            return __generator(this, function (_e) {
                switch (_e.label) {
                    case 0: return [4 /*yield*/, this.roots];
                    case 1:
                        _e.sent();
                        if (!this.opened) return [3 /*break*/, 15];
                        _e.label = 2;
                    case 2:
                        _e.trys.push([2, 13, 14, 15]);
                        _a = __values(this._roots), _b = _a.next();
                        _e.label = 3;
                    case 3:
                        if (!!_b.done) return [3 /*break*/, 12];
                        root = _b.value;
                        uri = root.resource;
                        _e.label = 4;
                    case 4:
                        _e.trys.push([4, 9, 10, 11]);
                        paths_1 = (e_3 = void 0, __values(paths)), paths_1_1 = paths_1.next();
                        _e.label = 5;
                    case 5:
                        if (!!paths_1_1.done) return [3 /*break*/, 8];
                        path = paths_1_1.value;
                        fileUri = uri.resolve(path);
                        return [4 /*yield*/, this.fileService.exists(fileUri)];
                    case 6:
                        exists = _e.sent();
                        if (exists) {
                            return [2 /*return*/, exists];
                        }
                        _e.label = 7;
                    case 7:
                        paths_1_1 = paths_1.next();
                        return [3 /*break*/, 5];
                    case 8: return [3 /*break*/, 11];
                    case 9:
                        e_3_1 = _e.sent();
                        e_3 = { error: e_3_1 };
                        return [3 /*break*/, 11];
                    case 10:
                        try {
                            if (paths_1_1 && !paths_1_1.done && (_d = paths_1.return)) _d.call(paths_1);
                        }
                        finally { if (e_3) throw e_3.error; }
                        return [7 /*endfinally*/];
                    case 11:
                        _b = _a.next();
                        return [3 /*break*/, 3];
                    case 12: return [3 /*break*/, 15];
                    case 13:
                        e_4_1 = _e.sent();
                        e_4 = { error: e_4_1 };
                        return [3 /*break*/, 15];
                    case 14:
                        try {
                            if (_b && !_b.done && (_c = _a.return)) _c.call(_a);
                        }
                        finally { if (e_4) throw e_4.error; }
                        return [7 /*endfinally*/];
                    case 15: return [2 /*return*/, false];
                }
            });
        });
    };
    Object.defineProperty(FxdkWorkspaceService.prototype, "saved", {
        get: function () {
            return !!this._workspace && !this._workspace.isDirectory;
        },
        enumerable: false,
        configurable: true
    });
    /**
     * Save workspace data into a file
     * @param uri URI or FileStat of the workspace file
     */
    FxdkWorkspaceService.prototype.save = function (uri) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/];
            });
        });
    };
    FxdkWorkspaceService.prototype.watchRoots = function () {
        return __awaiter(this, void 0, void 0, function () {
            var rootUris, _a, _b, _c, uri, watcher, _d, _e, root;
            var e_5, _f, e_6, _g;
            return __generator(this, function (_h) {
                rootUris = new Set(this._roots.map(function (r) { return r.resource.toString(); }));
                try {
                    for (_a = __values(this.rootWatchers.entries()), _b = _a.next(); !_b.done; _b = _a.next()) {
                        _c = __read(_b.value, 2), uri = _c[0], watcher = _c[1];
                        if (!rootUris.has(uri)) {
                            watcher.dispose();
                        }
                    }
                }
                catch (e_5_1) { e_5 = { error: e_5_1 }; }
                finally {
                    try {
                        if (_b && !_b.done && (_f = _a.return)) _f.call(_a);
                    }
                    finally { if (e_5) throw e_5.error; }
                }
                try {
                    for (_d = __values(this._roots), _e = _d.next(); !_e.done; _e = _d.next()) {
                        root = _e.value;
                        this.watchRoot(root);
                    }
                }
                catch (e_6_1) { e_6 = { error: e_6_1 }; }
                finally {
                    try {
                        if (_e && !_e.done && (_g = _d.return)) _g.call(_d);
                    }
                    finally { if (e_6) throw e_6.error; }
                }
                return [2 /*return*/];
            });
        });
    };
    FxdkWorkspaceService.prototype.refreshRootWatchers = function () {
        return __awaiter(this, void 0, void 0, function () {
            var _a, _b, watcher;
            var e_7, _c;
            return __generator(this, function (_d) {
                switch (_d.label) {
                    case 0:
                        try {
                            for (_a = __values(this.rootWatchers.values()), _b = _a.next(); !_b.done; _b = _a.next()) {
                                watcher = _b.value;
                                watcher.dispose();
                            }
                        }
                        catch (e_7_1) { e_7 = { error: e_7_1 }; }
                        finally {
                            try {
                                if (_b && !_b.done && (_c = _a.return)) _c.call(_a);
                            }
                            finally { if (e_7) throw e_7.error; }
                        }
                        return [4 /*yield*/, this.watchRoots()];
                    case 1:
                        _d.sent();
                        return [2 /*return*/];
                }
            });
        });
    };
    FxdkWorkspaceService.prototype.watchRoot = function (root) {
        return __awaiter(this, void 0, void 0, function () {
            var uriStr, excludes, watcher;
            var _this = this;
            return __generator(this, function (_a) {
                uriStr = root.resource.toString();
                if (this.rootWatchers.has(uriStr)) {
                    return [2 /*return*/];
                }
                excludes = this.getExcludes(uriStr);
                watcher = this.fileService.watch(new uri_1.default(uriStr), {
                    recursive: true,
                    excludes: excludes
                });
                this.rootWatchers.set(uriStr, new core_1.DisposableCollection(watcher, core_1.Disposable.create(function () { return _this.rootWatchers.delete(uriStr); })));
                return [2 /*return*/];
            });
        });
    };
    FxdkWorkspaceService.prototype.getExcludes = function (uri) {
        var patterns = this.fsPreferences.get('files.watcherExclude', undefined, uri);
        return Object.keys(patterns).filter(function (pattern) { return patterns[pattern]; });
    };
    FxdkWorkspaceService.prototype.areWorkspaceRoots = function (uris) {
        if (!uris.length) {
            return false;
        }
        var rootUris = new Set(this.tryGetRoots().map(function (root) { return root.resource.toString(); }));
        return uris.every(function (uri) { return rootUris.has(uri.toString()); });
    };
    // Filler
    FxdkWorkspaceService.prototype.getWorkspaceRootUri = function () {
        return;
    };
    __decorate([
        inversify_1.inject(file_service_1.FileService),
        __metadata("design:type", file_service_1.FileService)
    ], FxdkWorkspaceService.prototype, "fileService", void 0);
    __decorate([
        inversify_1.inject(common_1.WorkspaceServer),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "server", void 0);
    __decorate([
        inversify_1.inject(window_service_1.WindowService),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "windowService", void 0);
    __decorate([
        inversify_1.inject(core_1.ILogger),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "logger", void 0);
    __decorate([
        inversify_1.inject(workspace_preferences_1.WorkspacePreferences),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "preferences", void 0);
    __decorate([
        inversify_1.inject(browser_2.PreferenceServiceImpl),
        __metadata("design:type", browser_2.PreferenceServiceImpl)
    ], FxdkWorkspaceService.prototype, "preferenceImpl", void 0);
    __decorate([
        inversify_1.inject(browser_2.PreferenceSchemaProvider),
        __metadata("design:type", browser_2.PreferenceSchemaProvider)
    ], FxdkWorkspaceService.prototype, "schemaProvider", void 0);
    __decorate([
        inversify_1.inject(env_variables_1.EnvVariablesServer),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "envVariableServer", void 0);
    __decorate([
        inversify_1.inject(core_1.MessageService),
        __metadata("design:type", core_1.MessageService)
    ], FxdkWorkspaceService.prototype, "messageService", void 0);
    __decorate([
        inversify_1.inject(browser_2.LabelProvider),
        __metadata("design:type", browser_2.LabelProvider)
    ], FxdkWorkspaceService.prototype, "labelProvider", void 0);
    __decorate([
        inversify_1.inject(browser_3.FileSystemPreferences),
        __metadata("design:type", Object)
    ], FxdkWorkspaceService.prototype, "fsPreferences", void 0);
    __decorate([
        inversify_1.postConstruct(),
        __metadata("design:type", Function),
        __metadata("design:paramtypes", []),
        __metadata("design:returntype", Promise)
    ], FxdkWorkspaceService.prototype, "init", null);
    FxdkWorkspaceService = __decorate([
        inversify_1.injectable()
    ], FxdkWorkspaceService);
    return FxdkWorkspaceService;
}());
exports.FxdkWorkspaceService = FxdkWorkspaceService;
var WorkspaceData;
(function (WorkspaceData) {
    var validateSchema = new Ajv().compile({
        type: 'object',
        properties: {
            folders: {
                description: 'Root folders in the workspace',
                type: 'array',
                items: {
                    type: 'object',
                    properties: {
                        path: {
                            type: 'string',
                        }
                    },
                    required: ['path']
                }
            },
            settings: {
                description: 'Workspace preferences',
                type: 'object'
            }
        },
        required: ['folders']
    });
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    function is(data) {
        return !!validateSchema(data);
    }
    WorkspaceData.is = is;
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    function buildWorkspaceData(folders, settings) {
        var roots = [];
        if (folders.length > 0) {
            if (typeof folders[0] !== 'string') {
                roots = folders.map(function (folder) { return folder.resource.toString(); });
            }
            else {
                roots = folders;
            }
        }
        var data = {
            folders: roots.map(function (folder) { return ({ path: folder }); })
        };
        if (settings) {
            data.settings = settings;
        }
        return data;
    }
    WorkspaceData.buildWorkspaceData = buildWorkspaceData;
    function transformToRelative(data, workspaceFile) {
        var e_8, _a;
        var folderUris = [];
        var workspaceFileUri = new uri_1.default(workspaceFile ? workspaceFile.resource.toString() : '').withScheme('file');
        try {
            for (var _b = __values(data.folders), _c = _b.next(); !_c.done; _c = _b.next()) {
                var path = _c.value.path;
                var folderUri = new uri_1.default(path).withScheme('file');
                var rel = workspaceFileUri.parent.relative(folderUri);
                if (rel) {
                    folderUris.push(rel.toString());
                }
                else {
                    folderUris.push(folderUri.toString());
                }
            }
        }
        catch (e_8_1) { e_8 = { error: e_8_1 }; }
        finally {
            try {
                if (_c && !_c.done && (_a = _b.return)) _a.call(_b);
            }
            finally { if (e_8) throw e_8.error; }
        }
        return buildWorkspaceData(folderUris, data.settings);
    }
    WorkspaceData.transformToRelative = transformToRelative;
    function transformToAbsolute(data, workspaceFile) {
        var e_9, _a;
        if (workspaceFile) {
            var folders = [];
            try {
                for (var _b = __values(data.folders), _c = _b.next(); !_c.done; _c = _b.next()) {
                    var folder = _c.value;
                    var path = folder.path;
                    if (path.startsWith('file:///')) {
                        folders.push(path);
                    }
                    else {
                        folders.push(workspaceFile.resource.withScheme('file').parent.resolve(path).toString());
                    }
                }
            }
            catch (e_9_1) { e_9 = { error: e_9_1 }; }
            finally {
                try {
                    if (_c && !_c.done && (_a = _b.return)) _a.call(_b);
                }
                finally { if (e_9) throw e_9.error; }
            }
            return Object.assign(data, buildWorkspaceData(folders, data.settings));
        }
        return data;
    }
    WorkspaceData.transformToAbsolute = transformToAbsolute;
})(WorkspaceData = exports.WorkspaceData || (exports.WorkspaceData = {}));
function rebindWorkspaceService(bind, rebind) {
    bind(FxdkWorkspaceService).toSelf().inSingletonScope();
    rebind(browser_1.WorkspaceService).toService(FxdkWorkspaceService);
}
exports.rebindWorkspaceService = rebindWorkspaceService;
//# sourceMappingURL=rebindWorkspaceService.js.map