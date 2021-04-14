import { injectable, interfaces } from 'inversify';
import { VSXApiVersionProviderImpl } from '@theia/vsx-registry/lib/node/vsx-api-version-provider-backend-impl';

@injectable()
export class FxdkVSXApiVersionProvider extends VSXApiVersionProviderImpl {
  getApiVersion(): string {
    return '1.52.0';
  }
}

export const rebindVSXApiVersionProvider = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  rebind(VSXApiVersionProviderImpl).to(FxdkVSXApiVersionProvider).inSingletonScope();
};
