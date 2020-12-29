import { ContainerModule, interfaces } from 'inversify';

import { PluginDeployerParticipant } from '@theia/plugin-ext/lib/common/plugin-protocol';
import { DiskFileSystemProvider } from '@theia/filesystem/lib/node/disk-file-system-provider';

import { rebindEnvVariablesServerImpl } from '../backend/rebindEnvVariablesServerImpl';
import { FxdkDiskFileSystemProvider } from './fxdk-disk-file-system-provider';
import { FxdkPluginDeployerParticipant } from './fxdk-plugin-deployer-participant';

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  rebindEnvVariablesServerImpl(bind, rebind);

  bind(FxdkDiskFileSystemProvider).toSelf().inSingletonScope();
  rebind(DiskFileSystemProvider).toService(FxdkDiskFileSystemProvider);

  bind(FxdkPluginDeployerParticipant).toSelf().inSingletonScope();
  bind(PluginDeployerParticipant).toService(FxdkPluginDeployerParticipant);
});
