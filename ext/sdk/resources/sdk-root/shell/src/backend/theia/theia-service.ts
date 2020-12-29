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

  async beforeAppStart() {
    await this.ensureTheiaDefaultSettings();

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

  async createDefaultProjectSettings(projectStoragePath: string) {
    const settingsPath = this.fsService.joinPath(projectStoragePath, 'theia-settings.json');
    const settings = {
      folders: [],
      settings: {},
    };

    await this.fsService.writeFile(settingsPath, JSON.stringify(settings, null, 2));
  }

  private async ensureTheiaDefaultSettings() {
    const settingsPath = this.fsService.joinPath(this.configService.theiaConfigPath, 'settings.json');

    if (await this.fsService.statSafe(settingsPath)) {
      return;
    }

    await this.fsService.writeFile(settingsPath, JSON.stringify({
      'workbench.colorTheme': 'FxDK Dark',
    }, null, 2));
  }
}
