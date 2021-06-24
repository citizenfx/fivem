import { ContainerModule, interfaces } from 'inversify';

import { PluginDeployerParticipant, PluginDeployerResolver } from '@theia/plugin-ext/lib/common/plugin-protocol';
import { DiskFileSystemProvider } from '@theia/filesystem/lib/node/disk-file-system-provider';

import { rebindEnvVariablesServerImpl } from '../backend/rebindEnvVariablesServerImpl';
import { FxdkDiskFileSystemProvider } from './fxdk-disk-file-system-provider';
import { FxdkPluginDeployerParticipant, FxdkPluginDeployerResolver } from './fxdk-plugin-deployer-participant';
import { rebindVSXApiVersionProvider } from './fxdk-vsx-api-version-provider';
import { rebindFilesystemWatcherService } from './rebindFilesystemWatcherService';

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  rebindFilesystemWatcherService(bind, rebind);
  rebindEnvVariablesServerImpl(bind, rebind);
  rebindVSXApiVersionProvider(bind, rebind);

  bind(FxdkDiskFileSystemProvider).toSelf().inSingletonScope();
  rebind(DiskFileSystemProvider).toService(FxdkDiskFileSystemProvider);

  bind(FxdkPluginDeployerParticipant).toSelf().inSingletonScope();
  bind(PluginDeployerParticipant).toService(FxdkPluginDeployerParticipant);

  bind(PluginDeployerResolver).to(FxdkPluginDeployerResolver).inSingletonScope();
});
