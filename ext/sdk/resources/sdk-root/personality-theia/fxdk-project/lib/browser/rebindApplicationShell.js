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
exports.rebindApplicationShell = exports.FxdkApplicationShell = void 0;
var inversify_1 = require("inversify");
var application_shell_1 = require("@theia/core/lib/browser/shell/application-shell");
var FxdkApplicationShell = /** @class */ (function (_super) {
    __extends(FxdkApplicationShell, _super);
    function FxdkApplicationShell() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    /**
       * Add a widget to the application shell. The given widget must have a unique `id` property,
       * which will be used as the DOM id.
       *
       * Widgets are removed from the shell by calling their `close` or `dispose` methods.
       *
       * Widgets added to the top area are not tracked regarding the _current_ and _active_ states.
       */
    FxdkApplicationShell.prototype.addWidget = function (widget, options) {
        if (options === void 0) { options = {}; }
        return __awaiter(this, void 0, void 0, function () {
            var ref, area, tabBar, addOptions, areaPanel, sideRef, sidePanelOptions;
            return __generator(this, function (_a) {
                if (!widget.id) {
                    console.error('Widgets added to the application shell must have a unique id property.');
                    return [2 /*return*/];
                }
                ref = options.ref;
                area = options.area || 'main';
                if (!ref && (area === 'main' || area === 'bottom')) {
                    tabBar = this.getTabBarFor(area);
                    ref = tabBar && tabBar.currentTitle && tabBar.currentTitle.owner || undefined;
                }
                // make sure that ref belongs to area
                area = ref && this.getAreaFor(ref) || area;
                addOptions = {};
                if (application_shell_1.ApplicationShell.isOpenToSideMode(options.mode)) {
                    areaPanel = area === 'main' ? this.mainPanel : area === 'bottom' ? this.bottomPanel : undefined;
                    sideRef = areaPanel && ref && (options.mode === 'open-to-left' ?
                        areaPanel.previousTabBarWidget(ref) :
                        areaPanel.nextTabBarWidget(ref));
                    if (sideRef) {
                        addOptions.ref = sideRef;
                    }
                    else {
                        addOptions.ref = ref;
                        addOptions.mode = options.mode === 'open-to-left' ? 'split-left' : 'split-right';
                    }
                }
                else {
                    addOptions.ref = ref;
                    addOptions.mode = options.mode;
                }
                sidePanelOptions = { rank: options.rank };
                switch (area) {
                    case 'main':
                        this.mainPanel.addWidget(widget, addOptions);
                        break;
                    case 'top':
                        this.topPanel.addWidget(widget);
                        break;
                    case 'bottom':
                        this.bottomPanel.addWidget(widget, addOptions);
                        break;
                    case 'left':
                        // Always add to the right panel
                        this.rightPanelHandler.addWidget(widget, sidePanelOptions);
                        break;
                    case 'right':
                        this.rightPanelHandler.addWidget(widget, sidePanelOptions);
                        break;
                    default:
                        throw new Error('Unexpected area: ' + options.area);
                }
                if (area !== 'top') {
                    this.track(widget);
                }
                return [2 /*return*/];
            });
        });
    };
    FxdkApplicationShell = __decorate([
        inversify_1.injectable()
    ], FxdkApplicationShell);
    return FxdkApplicationShell;
}(application_shell_1.ApplicationShell));
exports.FxdkApplicationShell = FxdkApplicationShell;
exports.rebindApplicationShell = function (bind, rebind) {
    bind(FxdkApplicationShell).toSelf().inSingletonScope();
    rebind(application_shell_1.ApplicationShell).toService(FxdkApplicationShell);
};
//# sourceMappingURL=rebindApplicationShell.js.map