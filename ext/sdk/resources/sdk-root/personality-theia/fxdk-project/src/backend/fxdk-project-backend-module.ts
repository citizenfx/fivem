import { ContainerModule, interfaces } from 'inversify';

import { DiskFileSystemProvider } from '@theia/filesystem/lib/node/disk-file-system-provider';

import { rebindEnvVariablesServerImpl } from '../backend/rebindEnvVariablesServerImpl';
import { FxdkDiskFileSystemProvider } from './fxdk-disk-file-system-provider';

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  rebindEnvVariablesServerImpl(bind, rebind);

  bind(FxdkDiskFileSystemProvider).toSelf().inSingletonScope();
  rebind(DiskFileSystemProvider).toService(FxdkDiskFileSystemProvider);
});
