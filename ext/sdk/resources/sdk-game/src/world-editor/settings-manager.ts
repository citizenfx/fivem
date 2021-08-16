import { WEApi } from "@sdk-root/backend/world-editor/world-editor-game-api";
import { WESettings } from "@sdk-root/backend/world-editor/world-editor-types";
import { onWEApi } from "./utils";

export const SettingsManager = new class SettingsManager {
  public settings: WESettings = {
    mouseSensetivity: 15,
    cameraAboveTheGround: true,
    showSelectionBoundingBox: true,

    playtestSpawnInVehicle: false,
    playtestVehicleName: '',
  };

  preinit() {
    onWEApi(WEApi.SettingsSet, (settings) => {
      this.settings = {
        ...this.settings,
        ...settings,
      };
    });
  }
};
