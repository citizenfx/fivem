import { inject, injectable } from "inversify";
import { AppContribution } from "backend/app/app-contribution";
import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";

@injectable()
export class TheiaService implements AppContribution {
  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(FsService)
  protected readonly fsService: FsService;

  getReferenceTheiaConfig(): Record<string, unknown> {
    return {
      'workbench.colorTheme': 'FxDK Dark',
      'Lua.workspace.library': {
        [this.configService.nativesDocluaPath.replace(/\\/g, '/')]: true,
      },
      'Lua.diagnostics.enable': false,
    };
  }

  async boot() {
    await this.ensureTheiaDefaultSettings();
  }

  async beforeAppStart() {
    await this.startTheiaBackend();
  }

  async createDefaultProjectSettings(projectStoragePath: string) {
    const settingsPath = this.fsService.joinPath(projectStoragePath, 'theia-settings.json');
    const settings = {
      folders: [],
      settings: {},
    };

    await this.fsService.writeFileJson(settingsPath, settings);
  }

  private async startTheiaBackend() {
    if (!this.configService.selfHosted) {
      return;
    }

    const theiaBackendPath = this.fsService.joinPath(
      this.configService.sdkRootTheia,
      'backend',
    );

    try {
      process.chdir(this.configService.sdkRootTheia);
      const fakeArgv = [...process.argv, '--plugins=local-dir:plugins'];

      await nativeRequire(theiaBackendPath)(this.configService.theiaBackendPort, 'localhost', fakeArgv);
    } catch (e) {
      console.log('personality-theia has failed to start');
      console.error(e);
    }
  }

  private async ensureTheiaDefaultSettings() {
    const settingsPath = this.fsService.joinPath(this.configService.theiaConfigPath, 'settings.json');
    const referenceConfig = this.getReferenceTheiaConfig();

    if (await this.fsService.statSafe(settingsPath)) {
      // Add missing bits
      let currentConfig: Record<string, unknown> = {};

      try {
        currentConfig = await this.fsService.readFileJson(settingsPath);
      } catch (e) {
        // welp, we'll just write reference then
      }

      Object.entries(referenceConfig).forEach(([key, value]) => {
        if (!(key in currentConfig)) {
          currentConfig[key] = value;
        }
      });

      await this.fsService.writeFileJson(settingsPath, currentConfig);

      return;
    }

    // Ensure folder exists
    await this.fsService.mkdirp(this.configService.theiaConfigPath);
    await this.fsService.writeFileJson(settingsPath, referenceConfig);
  }
}
