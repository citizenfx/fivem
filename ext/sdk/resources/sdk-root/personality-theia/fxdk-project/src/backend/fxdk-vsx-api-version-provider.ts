import { injectable, interfaces } from 'inversify';
import { VSXApiVersionProviderImpl } from '@theia/vsx-registry/lib/node/vsx-api-version-provider-backend-impl';
import { VSCODE_API_VERSION } from '../common/vscode-api-version';

@injectable()
export class FxdkVSXApiVersionProvider extends VSXApiVersionProviderImpl {
  getApiVersion(): string {
    return VSCODE_API_VERSION;
  }
}

export const rebindVSXApiVersionProvider = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  rebind(VSXApiVersionProviderImpl).to(FxdkVSXApiVersionProvider).inSingletonScope();
};
