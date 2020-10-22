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
exports.FxdkMenuCommandHandler = exports.FxdkMenuContribution = exports.FxdkCommands = exports.FxdkMenus = void 0;
var core_1 = require("@theia/core");
var editor_manager_1 = require("@theia/editor/lib/browser/editor-manager");
var editor_menu_1 = require("@theia/editor/lib/browser/editor-menu");
var inversify_1 = require("inversify");
var fxdk_data_service_1 = require("./fxdk-data-service");
function formatArrayOfFloats(arr) {
    return arr.map(function (coord) { return coord.toFixed(3); }).join(', ');
}
var FxdkMenus;
(function (FxdkMenus) {
    FxdkMenus.GAME = __spread(core_1.MAIN_MENU_BAR, ['1_game']);
    FxdkMenus.GAME_INSERTIONS = __spread(FxdkMenus.GAME, ['1_insertions']);
    FxdkMenus.GAME_TOGGLES = __spread(FxdkMenus.GAME, ['2_toggles']);
    FxdkMenus.GAME_CONTEXT = __spread(editor_menu_1.EDITOR_CONTEXT_MENU, ['0_game_context']);
    FxdkMenus.GAME_CONTEXT_INSERTIONS = __spread(FxdkMenus.GAME_CONTEXT, ['z_game_insertions']);
})(FxdkMenus = exports.FxdkMenus || (exports.FxdkMenus = {}));
var FxdkCommands;
(function (FxdkCommands) {
    var GAME_CATEGORY = 'Game';
    FxdkCommands.INSERT_CURRENT_POS = {
        id: 'fxdk.insertCurrentPos',
        category: GAME_CATEGORY,
        label: 'Insert player position',
    };
    FxdkCommands.INSERT_CURRENT_ROT = {
        id: 'fxdk.insertCurrentRot',
        category: GAME_CATEGORY,
        label: 'Insert player rotation',
    };
    FxdkCommands.INSERT_CURRENT_HEADING = {
        id: 'fxdk.insertCurrentHeading',
        category: GAME_CATEGORY,
        label: 'Insert player heading',
    };
})(FxdkCommands = exports.FxdkCommands || (exports.FxdkCommands = {}));
var FxdkMenuContribution = /** @class */ (function () {
    function FxdkMenuContribution() {
    }
    FxdkMenuContribution.prototype.registerMenus = function (registry) {
        /**
         * Bar menus
         */
        registry.registerSubmenu(FxdkMenus.GAME, 'Game');
        registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
            commandId: FxdkCommands.INSERT_CURRENT_POS.id,
        });
        registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
            commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
        });
        registry.registerMenuAction(FxdkMenus.GAME_INSERTIONS, {
            commandId: FxdkCommands.INSERT_CURRENT_HEADING.id,
        });
        registry.registerMenuAction(FxdkMenus.GAME_TOGGLES, {
            commandId: 'fxdkGameView:toggle',
            label: 'Toggle Game View',
        });
        /**
         * Context menus
         */
        registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
            commandId: FxdkCommands.INSERT_CURRENT_POS.id,
        });
        registry.registerMenuAction(FxdkMenus.GAME_CONTEXT_INSERTIONS, {
            commandId: FxdkCommands.INSERT_CURRENT_ROT.id,
        });
    };
    FxdkMenuContribution.prototype.registerCommands = function (registry) {
        var _this = this;
        registry.registerCommand(FxdkCommands.INSERT_CURRENT_POS, this.newEditorCommandHandler(function (editor) {
            var pos = _this.dataService.data['player_ped_pos'];
            if (!pos) {
                console.log('No pos yet');
                return;
            }
            editor.executeEdits([{
                    range: editor.selection,
                    newText: formatArrayOfFloats(pos),
                }]);
        }));
        registry.registerCommand(FxdkCommands.INSERT_CURRENT_ROT, this.newEditorCommandHandler(function (editor) {
            var rot = _this.dataService.data['player_ped_rot'];
            if (!rot) {
                console.log('No rot yet');
                return;
            }
            editor.executeEdits([{
                    range: editor.selection,
                    newText: formatArrayOfFloats(rot),
                }]);
        }));
        registry.registerCommand(FxdkCommands.INSERT_CURRENT_HEADING, this.newEditorCommandHandler(function (editor) {
            var heading = _this.dataService.data['player_ped_heading'];
            if (!heading) {
                console.log('No heading yet');
                return;
            }
            editor.executeEdits([{
                    range: editor.selection,
                    newText: heading.toFixed(3),
                }]);
        }));
    };
    FxdkMenuContribution.prototype.newCommandHandler = function (handler) {
        return new FxdkMenuCommandHandler(handler);
    };
    FxdkMenuContribution.prototype.newEditorCommandHandler = function (handler) {
        var _this = this;
        var commandHandler = function () {
            var _a;
            var editor = (_a = _this.editorManager.activeEditor) === null || _a === void 0 ? void 0 : _a.editor;
            if (!editor) {
                console.log('No active editor');
                return;
            }
            handler(editor);
        };
        return this.newCommandHandler(commandHandler);
    };
    __decorate([
        inversify_1.inject(fxdk_data_service_1.FxdkDataService),
        __metadata("design:type", fxdk_data_service_1.FxdkDataService)
    ], FxdkMenuContribution.prototype, "dataService", void 0);
    __decorate([
        inversify_1.inject(editor_manager_1.EditorManager),
        __metadata("design:type", editor_manager_1.EditorManager)
    ], FxdkMenuContribution.prototype, "editorManager", void 0);
    FxdkMenuContribution = __decorate([
        inversify_1.injectable()
    ], FxdkMenuContribution);
    return FxdkMenuContribution;
}());
exports.FxdkMenuContribution = FxdkMenuContribution;
var FxdkMenuCommandHandler = /** @class */ (function () {
    function FxdkMenuCommandHandler(handler) {
        this.handler = handler;
    }
    FxdkMenuCommandHandler.prototype.execute = function () {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        this.handler.apply(this, __spread(args));
    };
    return FxdkMenuCommandHandler;
}());
exports.FxdkMenuCommandHandler = FxdkMenuCommandHandler;
//# sourceMappingURL=fxdk-menu-contribution.js.map