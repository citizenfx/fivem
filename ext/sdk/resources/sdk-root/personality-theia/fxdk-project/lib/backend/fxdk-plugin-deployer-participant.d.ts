import { PluginTheiaEnvironment } from '@theia/plugin-ext/lib/main/common/plugin-theia-environment';
import { PluginDeployerParticipant, PluginDeployerStartContext } from '@theia/plugin-ext/lib/common/plugin-protocol';
export declare class FxdkPluginDeployerParticipant implements PluginDeployerParticipant {
    protected readonly environments: PluginTheiaEnvironment;
    onWillStart(context: PluginDeployerStartContext): Promise<void>;
}
//# sourceMappingURL=fxdk-plugin-deployer-participant.d.ts.map