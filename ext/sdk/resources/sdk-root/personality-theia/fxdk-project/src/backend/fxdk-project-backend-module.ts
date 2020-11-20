import { ContainerModule, interfaces } from 'inversify';

import { rebindEnvVariablesServerImpl } from '../backend/rebindEnvVariablesServerImpl';

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  rebindEnvVariablesServerImpl(bind, rebind);
});
