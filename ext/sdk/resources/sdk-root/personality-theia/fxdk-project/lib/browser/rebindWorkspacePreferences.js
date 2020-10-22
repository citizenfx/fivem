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
Object.defineProperty(exports, "__esModule", { value: true });
exports.rebindWorkspacePreferences = void 0;
var inversify_1 = require("inversify");
var workspace_preferences_1 = require("@theia/workspace/lib/browser/workspace-preferences");
var preferences_1 = require("@theia/core/lib/browser/preferences");
var FxdkPreferenceSchemaProvider = /** @class */ (function (_super) {
    __extends(FxdkPreferenceSchemaProvider, _super);
    function FxdkPreferenceSchemaProvider() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    FxdkPreferenceSchemaProvider.prototype.doSetSchema = function (schema) {
        // Disabling workspace preferences
        if (schema === workspace_preferences_1.workspacePreferenceSchema) {
            return [];
        }
        return _super.prototype.doSetSchema.call(this, schema);
    };
    FxdkPreferenceSchemaProvider = __decorate([
        inversify_1.injectable()
    ], FxdkPreferenceSchemaProvider);
    return FxdkPreferenceSchemaProvider;
}(preferences_1.PreferenceSchemaProvider));
function rebindWorkspacePreferences(rebind) {
    rebind(preferences_1.PreferenceSchemaProvider).to(FxdkPreferenceSchemaProvider).inSingletonScope();
}
exports.rebindWorkspacePreferences = rebindWorkspacePreferences;
//# sourceMappingURL=rebindWorkspacePreferences.js.map