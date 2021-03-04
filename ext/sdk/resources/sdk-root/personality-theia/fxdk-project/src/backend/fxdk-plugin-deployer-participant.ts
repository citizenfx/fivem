import * as fs from 'fs';
import * as path from 'path';
import { injectable } from 'inversify';

import {
  PluginDeployerParticipant,
  PluginDeployerStartContext,
  PluginDeployerResolver,
  PluginDeployerResolverContext,
} from '@theia/plugin-ext/lib/common/plugin-protocol';

@injectable()
export class FxdkPluginDeployerParticipant implements PluginDeployerParticipant {
  async onWillStart(context: PluginDeployerStartContext): Promise<void> {
    context.systemEntries.push('fxdk-plugins');
  }
}

@injectable()
export class FxdkPluginDeployerResolver implements PluginDeployerResolver {
  async resolve(pluginResolverContext: PluginDeployerResolverContext): Promise<void> {
    const localAppData = process.env.LOCALAPPDATA;
    if (!localAppData) {
      console.error('No LOCALAPPDATA env var');

      return;
    }

    const fxdkPluginsPath = path.join(localAppData, 'citizenfx/sdk-personality-theia-plugins');
    const fxdkPluginsPathChildren = await fs.promises.readdir(fxdkPluginsPath);

    await Promise.all(fxdkPluginsPathChildren.map(async (pluginId) => {
      const pluginPath = path.join(fxdkPluginsPath, pluginId);
      const stat = await fs.promises.stat(pluginPath);

      if (stat.isDirectory()) {
        pluginResolverContext.addPlugin(pluginId, pluginPath);
      }
    }));
  }

  accept(pluginId: string): boolean {
    return pluginId === 'fxdk-plugins';
  }
}
