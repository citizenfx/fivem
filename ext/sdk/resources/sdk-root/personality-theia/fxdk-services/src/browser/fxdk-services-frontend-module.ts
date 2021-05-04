import { ContainerModule, interfaces } from 'inversify';

import { FxdkDataService } from './fxdk-data-service';

export default new ContainerModule((bind: interfaces.Bind, unbind: interfaces.Unbind, isBound: interfaces.IsBound, rebind: interfaces.Rebind) => {
  bind(FxdkDataService).toSelf().inSingletonScope();
});
