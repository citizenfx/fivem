import { FXCodeImportConfig, IKeybinding } from "backend/fxcode/fxcode-types";
import { makeAutoObservable } from "mobx";

export class ImportConfigDraft {
  private excludedSettings: Record<string, boolean> = {};
  private excludedKeybindings: Record<number, boolean> = {};
  private excludedExtensions: Record<string, boolean> = {};

  constructor(
    public config: FXCodeImportConfig,
  ) {
    makeAutoObservable(this);
  }

  get allSettingsIncluded(): boolean {
    return !Object.values(this.excludedSettings).find((excluded) => excluded);
  }
  toggleAllSettingsIncluded() {
    if (this.allSettingsIncluded) {
      this.excludedSettings = Object.keys(this.config.settings).reduce((acc, key) => {
        acc[key] = true;

        return acc;
      }, {});
    } else {
      this.excludedSettings = {};
    }
  }
  isSettingIncluded(key: string) {
    return !this.excludedSettings[key];
  }
  setSettingIncluded(key: string, included: boolean) {
    this.excludedSettings[key] = !included;
  }

  get allKeybindingsIncluded(): boolean {
    return !Object.values(this.excludedKeybindings).find((excluded) => excluded);
  }
  toggleAllKeybindingsIncluded() {
    if (this.allKeybindingsIncluded) {
      this.excludedKeybindings = Object.keys(this.config.keybindings).reduce((acc, key) => {
        acc[key] = true;

        return acc;
      }, {});
    } else {
      this.excludedKeybindings = {};
    }
  }
  isKeybindIncluded(index: number) {
    return !this.excludedKeybindings[index];
  }
  setKeybindingIncluded(index: number, included: boolean) {
    this.excludedKeybindings[index] = !included;
  }

  get allExtensionsIncluded(): boolean {
    return !Object.values(this.excludedExtensions).find((excluded) => excluded);
  }
  toggleAllExtensionsIncluded() {
    if (this.allExtensionsIncluded) {
      this.excludedExtensions = Object.keys(this.config.extensions).reduce((acc, key) => {
        acc[key] = true;

        return acc;
      }, {});
    } else {
      this.excludedExtensions = {};
    }
  }
  isExtensionIncluded(id: string) {
    return !this.excludedExtensions[id];
  }
  setExtensionIncluded(id: string, included: boolean) {
    this.excludedExtensions[id] = !included;
  }

  getConfig(): FXCodeImportConfig {
    const settings: Record<string, unknown> = {};
    for (const [key, value] of Object.entries(this.config.settings)) {
      if (!this.excludedSettings[key]) {
        settings[key] = value;
      }
    }

    const keybindings: IKeybinding[] = [];
    this.config.keybindings.forEach((keybinding, index) => {
      if (!this.excludedKeybindings[index]) {
        keybindings.push(keybinding);
      }
    });

    const extensions: Record<string, boolean> = {
      ...this.config.extensions,
      ...this.excludedExtensions,
    };

    return {
      settings,
      keybindings,
      extensions,
    };
  }

  getExtensionIds(): string[] {
    return Object.entries({
      ...this.config.extensions,
      ...this.excludedExtensions,
    }).filter(([, install]) => install).map(([id]) => id);
  }
}
