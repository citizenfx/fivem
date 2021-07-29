import { WESettings } from "./map-types";

export const SettingsManager = new class SettingsManager {
  public settings: WESettings = {
    mouseSensetivity: 15,
    showSelectionBoundingBox: true,
  };

  preinit() {
    on('we:settings', (settings: string) => {
      console.log('settings', settings);

      this.settings = {
        ...this.settings,
        ...JSON.parse(settings),
      };
    });
  }
};
