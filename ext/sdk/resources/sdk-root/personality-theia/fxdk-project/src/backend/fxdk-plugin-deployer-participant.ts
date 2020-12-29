import * as path from 'path';
import { injectable, inject } from 'inversify';

import URI from '@theia/core/lib/common/uri';
import { PluginTheiaEnvironment } from '@theia/plugin-ext/lib/main/common/plugin-theia-environment';
import { PluginDeployerParticipant, PluginDeployerStartContext } from '@theia/plugin-ext/lib/common/plugin-protocol';

const invariant = (s: string | void, err: string) => {
  if (!s) {
    throw new Error(err);
  }

  return s;
}

@injectable()
export class FxdkPluginDeployerParticipant implements PluginDeployerParticipant {
  @inject(PluginTheiaEnvironment)
  protected readonly environments: PluginTheiaEnvironment;

  async onWillStart(context: PluginDeployerStartContext): Promise<void> {
    const localAppData = invariant(process.env.LOCALAPPDATA, 'No LOCALAPPDATA env var');
    const fxdkTheiaPluginsDir = new URI(path.join(localAppData, 'citizenfx/sdk-personality-theia-plugins'));

    context.userEntries.push(fxdkTheiaPluginsDir.withScheme('local-dir').toString());
  }
}
