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
Object.defineProperty(exports, "__esModule", { value: true });
exports.rebindEnvVariablesServerImpl = exports.FxdkEnvVariablesServerImpl = void 0;
var path_1 = require("path");
var os_1 = require("os");
var child_process_1 = require("child_process");
var inversify_1 = require("inversify");
var env_variables_1 = require("@theia/core/lib/common/env-variables");
var file_uri_1 = require("@theia/core/lib/node/file-uri");
var invariant = function (s, err) {
    if (!s) {
        throw new Error(err);
    }
    return s;
};
var FxdkEnvVariablesServerImpl = /** @class */ (function () {
    function FxdkEnvVariablesServerImpl() {
        var _this = this;
        this.envs = {};
        this.homeDirUri = file_uri_1.FileUri.create(os_1.homedir()).toString();
        this.configDirUri = this.createConfigDirUri();
        this.configDirUri.then(function (configDirUri) { return console.log("Configuration directory URI: '" + configDirUri + "'"); });
        var prEnv = process.env;
        Object.keys(prEnv).forEach(function (key) {
            var keyName = key.toLowerCase();
            _this.envs[keyName] = { 'name': keyName, 'value': prEnv[key] };
        });
    }
    FxdkEnvVariablesServerImpl.prototype.createConfigDirUri = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, file_uri_1.FileUri.create(path_1.join(invariant(process.env.LOCALAPPDATA, 'No LOCALAPPDATA env var'), 'citizenfx/sdk-personality-theia')).toString()];
            });
        });
    };
    FxdkEnvVariablesServerImpl.prototype.getExecPath = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, process.execPath];
            });
        });
    };
    FxdkEnvVariablesServerImpl.prototype.getVariables = function () {
        return __awaiter(this, void 0, void 0, function () {
            var _this = this;
            return __generator(this, function (_a) {
                return [2 /*return*/, Object.keys(this.envs).map(function (key) { return _this.envs[key]; })];
            });
        });
    };
    FxdkEnvVariablesServerImpl.prototype.getValue = function (key) {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, this.envs[key.toLowerCase()]];
            });
        });
    };
    FxdkEnvVariablesServerImpl.prototype.getConfigDirUri = function () {
        return this.configDirUri;
    };
    FxdkEnvVariablesServerImpl.prototype.getHomeDirUri = function () {
        return __awaiter(this, void 0, void 0, function () {
            return __generator(this, function (_a) {
                return [2 /*return*/, this.homeDirUri];
            });
        });
    };
    /**
     * Workaround of not working with our uv-thingie drivelist package
     *
     * As FxDK only available on widnows we can just use wmic command,
     * just like we do in sdk itself
     */
    FxdkEnvVariablesServerImpl.prototype.getDrives = function () {
        return __awaiter(this, void 0, void 0, function () {
            var uris, drives;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        uris = [];
                        return [4 /*yield*/, new Promise(function (resolve) {
                                child_process_1.exec('wmic logicaldisk get name', { windowsHide: true }, function (error, stdout) {
                                    resolve(stdout.split('\n')
                                        .filter(function (value) { return /[A-Za-z]:/.test(value); })
                                        .map(function (value) { return value.trim(); }));
                                });
                            })];
                    case 1:
                        drives = _a.sent();
                        drives.forEach(function (drive) { return uris.push(file_uri_1.FileUri.create(drive).toString()); });
                        return [2 /*return*/, uris];
                }
            });
        });
    };
    FxdkEnvVariablesServerImpl = __decorate([
        inversify_1.injectable(),
        __metadata("design:paramtypes", [])
    ], FxdkEnvVariablesServerImpl);
    return FxdkEnvVariablesServerImpl;
}());
exports.FxdkEnvVariablesServerImpl = FxdkEnvVariablesServerImpl;
exports.rebindEnvVariablesServerImpl = function (bind, rebind) {
    rebind(env_variables_1.EnvVariablesServer).to(FxdkEnvVariablesServerImpl).inSingletonScope();
};
//# sourceMappingURL=rebindEnvVariablesServerImpl.js.map