import * as os from 'os';
import { ApiContribution } from "backend/api/api.extensions";
import { AppContribution } from "backend/app/app.extensions";
import { FsService } from "backend/fs/fs-service";
import { inject, injectable } from "inversify";
import { FXCodeImportConfig, IKeybinding } from './fxcode-types';
import { handlesClientCallbackEvent } from 'backend/api/api-decorators';
import { fxcodeApi } from './fxcode-requests';
import { ConfigService } from 'backend/config-service';

@injectable()
export class FXCodeImporter implements ApiContribution, AppContribution {
  getId() {
    return 'FXCodeImporter';
  }

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  private roamingCodePath = '';
  private userCodePath = '';

  boot() {
    const appDataPath = process.env.APPDATA;
    if (!appDataPath) {
      return;
    }

    this.roamingCodePath = this.fsService.joinPath(appDataPath, 'Code/User');
    this.userCodePath = this.fsService.joinPath(os.homedir(), '.vscode');
  }

  @handlesClientCallbackEvent(fxcodeApi.getImportConfig)
  async getMigrationConfig(): Promise<FXCodeImportConfig | undefined> {
    const [settings, keybindings, extensions] = await Promise.all([
      this.getSettings(),
      this.getKeybindings(),
      this.getExtensionIds(),
    ]);

    if (!Object.keys(settings).length && !keybindings.length && !Object.keys(extensions).length) {
      return;
    }

    return {
      settings,
      keybindings,
      extensions,
    };
  }

  @handlesClientCallbackEvent(fxcodeApi.applyImportConfig)
  async applyImportConfig(config: FXCodeImportConfig): Promise<void> {
    const settingsPath = this.fsService.joinPath(this.configService.fxcodeDataPath, 'user/User/settings.json');
    if (!(await this.fsService.statSafe(settingsPath))) {
      await this.fsService.writeFileJson(settingsPath, config.settings);
    }

    const keybindingsPath = this.fsService.joinPath(this.configService.fxcodeDataPath, 'user/User/keybindings.json');
    if (!(await this.fsService.statSafe(keybindingsPath))) {
      await this.fsService.writeFileJson(keybindingsPath, config.keybindings);
    }
  }

  private async getSettings(): Promise<Record<string, unknown>> {
    if (!this.roamingCodePath) {
      return {};
    }

    const settingsPath = this.fsService.joinPath(this.roamingCodePath, 'settings.json');
    if (!(await this.fsService.statSafe(settingsPath))) {
      return {};
    }

    try {
      const content = await this.fsService.readFileJsonc(settingsPath);

      if (typeof content === 'object' && content !== null) {
        return content as any;
      }

      return {};
    } catch (e) {
      return {};
    }
  }

  private async getKeybindings(): Promise<IKeybinding[]> {
    if (!this.roamingCodePath) {
      return [];
    }

    const keybindingsPath = this.fsService.joinPath(this.roamingCodePath, 'keybindings.json');
    if (!(await this.fsService.statSafe(keybindingsPath))) {
      return [];
    }

    try {
      const content = await this.fsService.readFileJsonc(keybindingsPath);

      if (!Array.isArray(content)) {
        return [];
      }

      return content;
    } catch (e) {
      return [];
    }
  }

  private async getExtensionIds(): Promise<Record<string, boolean>> {
    if (!this.userCodePath) {
      return {};
    }

    const extensionsPath = this.fsService.joinPath(this.userCodePath, 'extensions');
    const extensionsPathStat = await this.fsService.statSafe(extensionsPath);
    if (!extensionsPathStat?.isDirectory()) {
      return {};
    }

    let obsoleteExtensions: Record<string, boolean> = {};

    const obsoletePath = this.fsService.joinPath(extensionsPath, '.obsolete');
    if (await this.fsService.statSafe(obsoletePath)) {
      try {
        obsoleteExtensions = await this.fsService.readFileJson(obsoletePath);
      } catch (e) {
        // welp, we tried
      }
    }

    const extensionEntries = await this.fsService.readdir(extensionsPath);

    const extensionIds = await Promise.all(
      extensionEntries
        .filter((name) => {
          if (name === '.obsolete') {
            return false;
          }

          if (obsoleteExtensions[name]) {
            return false;
          }

          return true;
        })
        .map((name) => this.getExtensionId(this.fsService.joinPath(extensionsPath, name))),
    );

    return extensionIds.reduce((acc, id) => {
      if (!id) {
        return acc;
      }

      acc[id] = true;

      return acc;
    }, {});
  }

  private async getExtensionId(extensionPath: string): Promise<string | void> {
    const vsixmanifestPath = this.fsService.joinPath(extensionPath, '.vsixmanifest');
    if (!(await this.fsService.statSafe(vsixmanifestPath))) {
      return;
    }

    try {
      const { PackageManifest: { Metadata: { Identity } } } = await this.fsService.readFileXML(vsixmanifestPath, { ignoreAttributes: false });

      return `${Identity['@_Publisher']}.${Identity['@_Id']}`.toLowerCase();
    } catch (e) {
      return;
    }
  }
}
