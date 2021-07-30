import { WESettings } from "backend/world-editor/world-editor-types";
import deepmerge from "deepmerge";
import { makeAutoObservable, reaction } from "mobx";
import { clamp } from "shared/math";
import { debounce } from "shared/utils";
import { onWindowEvent } from "utils/windowMessages";

const DEFAULT_SETTINGS: WESettings = {
  mouseSensetivity: 15,
  cameraAboveTheGround: true,
  showSelectionBoundingBox: true,

  playtestSpawnInVehicle: false,
  playtestVehicleName: '',
};

function getStoredSettings(): WESettings {
  if (localStorage['we:settings']) {
    try {
      return deepmerge({ ...DEFAULT_SETTINGS }, JSON.parse(localStorage['we:settings']));
    } catch (e) {
      // welp
    }
  }

  return { ...DEFAULT_SETTINGS };
}

export const WESettingsState = new class WESettingsState {
  public settings: WESettings = getStoredSettings();

  constructor() {
    makeAutoObservable(this);

    reaction(
      () => ({ ...this.settings }),
      this.handleSettingsChange,
    );

    onWindowEvent('we:ready', () => {
      sendGameClientEvent('we:settings', JSON.stringify(this.settings));
    });
  }

  setMouseSensetivity(n: number) {
    this.settings.mouseSensetivity = clamp(n, 1, 100);
  }

  setShowSelectionBoundingBox(show: boolean) {
    this.settings.showSelectionBoundingBox = show;
  }

  setCameraAboveTheGround(above: boolean) {
    this.settings.cameraAboveTheGround = above;
  }

  setPlaytestSpawnInVehicle(spawnInVehicle: boolean) {
    this.settings.playtestSpawnInVehicle = spawnInVehicle;
  }

  setPlaytestVehicleName(name: string) {
    this.settings.playtestVehicleName = name;
  }

  private handleSettingsChange = () => {
    sendGameClientEvent('we:settings', JSON.stringify(this.settings));

    this.updateLocalStorage();
  };

  private updateLocalStorage = debounce(() => {
    localStorage['we:settings'] = JSON.stringify(this.settings);
  }, 100);
};
