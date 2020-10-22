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
var __assign = (this && this.__assign) || function () {
    __assign = Object.assign || function(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
                t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
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
exports.FxdkDiskFileSystemProvider = void 0;
var inversify_1 = require("inversify");
var disk_file_system_provider_1 = require("@theia/filesystem/lib/node/disk-file-system-provider");
var files_1 = require("@theia/filesystem/lib/common/files");
function toFileSystemProviderError(error) {
    if (error instanceof files_1.FileSystemProviderError) {
        return error; // avoid double conversion
    }
    var code;
    switch (error.code) {
        case 'ENOENT':
            code = files_1.FileSystemProviderErrorCode.FileNotFound;
            break;
        case 'EISDIR':
            code = files_1.FileSystemProviderErrorCode.FileIsADirectory;
            break;
        case 'ENOTDIR':
            code = files_1.FileSystemProviderErrorCode.FileNotADirectory;
            break;
        case 'EEXIST':
            code = files_1.FileSystemProviderErrorCode.FileExists;
            break;
        case 'EPERM':
        case 'EACCES':
            code = files_1.FileSystemProviderErrorCode.NoPermissions;
            break;
        default:
            code = files_1.FileSystemProviderErrorCode.Unknown;
    }
    return files_1.createFileSystemProviderError(error, code);
}
var FxdkDiskFileSystemProvider = /** @class */ (function (_super) {
    __extends(FxdkDiskFileSystemProvider, _super);
    function FxdkDiskFileSystemProvider() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    FxdkDiskFileSystemProvider.prototype.updateFile = function (resource, changes, opts) {
        return __awaiter(this, void 0, void 0, function () {
            var stats, e_1;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0:
                        if (!(changes.length === 0)) return [3 /*break*/, 4];
                        _a.label = 1;
                    case 1:
                        _a.trys.push([1, 3, , 4]);
                        return [4 /*yield*/, this.stat(resource)];
                    case 2:
                        stats = _a.sent();
                        return [2 /*return*/, __assign(__assign({}, stats), { encoding: opts.readEncoding || opts.writeEncoding || 'utf8' })];
                    case 3:
                        e_1 = _a.sent();
                        throw toFileSystemProviderError(e_1);
                    case 4: return [2 /*return*/, _super.prototype.updateFile.call(this, resource, changes, opts)];
                }
            });
        });
    };
    FxdkDiskFileSystemProvider = __decorate([
        inversify_1.injectable()
    ], FxdkDiskFileSystemProvider);
    return FxdkDiskFileSystemProvider;
}(disk_file_system_provider_1.DiskFileSystemProvider));
exports.FxdkDiskFileSystemProvider = FxdkDiskFileSystemProvider;
//# sourceMappingURL=fxdk-disk-file-system-provider.js.map