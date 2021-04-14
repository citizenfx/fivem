import { injectable, interfaces, inject } from 'inversify';
import { VSXEnvironment } from '@theia/vsx-registry/lib/common/vsx-environment';
import { VSCODE_API_VERSION } from '../common/vscode-api-version';

@injectable()
export class FxdkVSXEnvironment extends VSXEnvironment {
  async getVscodeApiVersion(): Promise<string> {
    return Promise.resolve(VSCODE_API_VERSION);
  }
}

export const rebindVsxEnvironment = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  rebind(VSXEnvironment).to(FxdkVSXEnvironment).inSingletonScope();
};
