import { WESettings } from "./map-types";

export const SettingsManager = new class SettingsManager {
  public settings: WESettings = {
    mouseSensetivity: 15,
    cameraAboveTheGround: true,
    showSelectionBoundingBox: true,

    playtestSpawnInVehicle: false,
    playtestVehicleName: '',
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
