"use strict";
var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.FxdkDataService = void 0;
var inversify_1 = require("inversify");
var FxdkDataService = /** @class */ (function () {
    function FxdkDataService() {
        this.data = {};
    }
    FxdkDataService = __decorate([
        inversify_1.injectable()
    ], FxdkDataService);
    return FxdkDataService;
}());
exports.FxdkDataService = FxdkDataService;
//# sourceMappingURL=fxdk-data-service.js.map