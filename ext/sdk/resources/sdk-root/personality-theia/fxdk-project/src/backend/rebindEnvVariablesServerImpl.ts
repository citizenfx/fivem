import { join } from 'path';
import { homedir } from 'os';
import { exec } from 'child_process';
import { injectable, interfaces } from 'inversify';
import { EnvVariable, EnvVariablesServer } from '@theia/core/lib/common/env-variables';
import { FileUri } from '@theia/core/lib/node/file-uri';

const invariant = (s: string | void, err: string) => {
  if (!s) {
    throw new Error(err);
  }

  return s;
}

@injectable()
export class FxdkEnvVariablesServerImpl implements EnvVariablesServer {

  protected readonly envs: { [key: string]: EnvVariable } = {};
  protected readonly homeDirUri = FileUri.create(homedir()).toString();
  protected readonly configDirUri: Promise<string>;

  constructor() {
    this.configDirUri = this.createConfigDirUri();
    this.configDirUri.then(configDirUri => console.log(`Configuration directory URI: '${configDirUri}'`));
    const prEnv = process.env;
    Object.keys(prEnv).forEach((key: string) => {
      let keyName = key.toLowerCase();

      this.envs[keyName] = { 'name': keyName, 'value': prEnv[key] };
    });
  }

  protected async createConfigDirUri(): Promise<string> {
    return FileUri.create(join(invariant(process.env.LOCALAPPDATA, 'No LOCALAPPDATA env var'), 'citizenfx/sdk-personality-theia')).toString();
  }

  async getExecPath(): Promise<string> {
    return process.execPath;
  }

  async getVariables(): Promise<EnvVariable[]> {
    return Object.keys(this.envs).map(key => this.envs[key]);
  }

  async getValue(key: string): Promise<EnvVariable | undefined> {
    return this.envs[key.toLowerCase()];
  }

  getConfigDirUri(): Promise<string> {
    return this.configDirUri;
  }

  async getHomeDirUri(): Promise<string> {
    return this.homeDirUri;
  }

  /**
   * Workaround of not working with our uv-thingie drivelist package
   *
   * As FxDK only available on widnows we can just use wmic command,
   * just like we do in sdk itself
   */
  async getDrives(): Promise<string[]> {
    const uris: string[] = [];

    const drives = await new Promise<string[]>((resolve) => {
      exec('wmic logicaldisk get name', { windowsHide: true }, (error, stdout) => {
        resolve(
          stdout.split('\n')
            .filter(value => /[A-Za-z]:/.test(value))
            .map(value => value.trim()),
        );
      });
    });

    drives.forEach((drive) => uris.push(FileUri.create(drive).toString()));

    return uris;
  }
}

export const rebindEnvVariablesServerImpl = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  rebind(EnvVariablesServer).to(FxdkEnvVariablesServerImpl).inSingletonScope();
};
