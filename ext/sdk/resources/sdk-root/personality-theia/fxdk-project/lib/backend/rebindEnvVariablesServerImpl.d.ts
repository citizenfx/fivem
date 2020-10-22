import { interfaces } from 'inversify';
import { EnvVariable, EnvVariablesServer } from '@theia/core/lib/common/env-variables';
export declare class FxdkEnvVariablesServerImpl implements EnvVariablesServer {
    protected readonly envs: {
        [key: string]: EnvVariable;
    };
    protected readonly homeDirUri: string;
    protected readonly configDirUri: Promise<string>;
    constructor();
    protected createConfigDirUri(): Promise<string>;
    getExecPath(): Promise<string>;
    getVariables(): Promise<EnvVariable[]>;
    getValue(key: string): Promise<EnvVariable | undefined>;
    getConfigDirUri(): Promise<string>;
    getHomeDirUri(): Promise<string>;
    /**
     * Workaround of not working with our uv-thingie drivelist package
     *
     * As FxDK only available on widnows we can just use wmic command,
     * just like we do in sdk itself
     */
    getDrives(): Promise<string[]>;
}
export declare const rebindEnvVariablesServerImpl: (bind: interfaces.Bind, rebind: interfaces.Rebind) => void;
//# sourceMappingURL=rebindEnvVariablesServerImpl.d.ts.map