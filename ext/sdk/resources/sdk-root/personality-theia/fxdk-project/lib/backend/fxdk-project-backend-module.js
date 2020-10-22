"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var inversify_1 = require("inversify");
var plugin_protocol_1 = require("@theia/plugin-ext/lib/common/plugin-protocol");
var disk_file_system_provider_1 = require("@theia/filesystem/lib/node/disk-file-system-provider");
var rebindEnvVariablesServerImpl_1 = require("../backend/rebindEnvVariablesServerImpl");
var fxdk_disk_file_system_provider_1 = require("./fxdk-disk-file-system-provider");
var fxdk_plugin_deployer_participant_1 = require("./fxdk-plugin-deployer-participant");
exports.default = new inversify_1.ContainerModule(function (bind, unbind, isBound, rebind) {
    rebindEnvVariablesServerImpl_1.rebindEnvVariablesServerImpl(bind, rebind);
    bind(fxdk_disk_file_system_provider_1.FxdkDiskFileSystemProvider).toSelf().inSingletonScope();
    rebind(disk_file_system_provider_1.DiskFileSystemProvider).toService(fxdk_disk_file_system_provider_1.FxdkDiskFileSystemProvider);
    bind(fxdk_plugin_deployer_participant_1.FxdkPluginDeployerParticipant).toSelf().inSingletonScope();
    bind(plugin_protocol_1.PluginDeployerParticipant).toService(fxdk_plugin_deployer_participant_1.FxdkPluginDeployerParticipant);
});
//# sourceMappingURL=fxdk-project-backend-module.js.map