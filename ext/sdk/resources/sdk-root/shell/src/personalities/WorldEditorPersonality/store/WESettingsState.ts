import { WEApi } from "backend/world-editor/world-editor-game-api";
import { WESettings } from "backend/world-editor/world-editor-types";
import deepmerge from "deepmerge";
import { makeAutoObservable } from "mobx";
import { clamp } from "shared/math";
import { LocalStorageValue } from "store/generic/LocalStorageValue";
import { invokeWEApi, onWEApi } from "../we-api-utils";

const DEFAULT_SETTINGS: WESettings = {
  mouseSensetivity: 15,
  cameraAboveTheGround: true,
  showSelectionBoundingBox: true,

  playtestSpawnInVehicle: false,
  playtestVehicleName: '',
};

export const WESettingsState = new class WESettingsState {
  private settingsVault = new LocalStorageValue({
    key: 'we:settings',
    defaultValue: DEFAULT_SETTINGS,
    readTransform(settings) {
      return deepmerge({ ...DEFAULT_SETTINGS }, settings);
    },
  });

  public get settings(): WESettings {
    return this.settingsVault.get();
  }

  constructor() {
    makeAutoObservable(this);

    onWEApi(WEApi.Ready, () => {
      invokeWEApi(WEApi.SettingsSet, this.settings);
    });
  }

  setMouseSensetivity(n: number) {
    this.settings.mouseSensetivity = clamp(n, 1, 100);

    this.handleSettingsChange();
  }

  setShowSelectionBoundingBox(show: boolean) {
    this.settings.showSelectionBoundingBox = show;

    this.handleSettingsChange();
  }

  setCameraAboveTheGround(above: boolean) {
    this.settings.cameraAboveTheGround = above;

    this.handleSettingsChange();
  }

  setPlaytestSpawnInVehicle(spawnInVehicle: boolean) {
    this.settings.playtestSpawnInVehicle = spawnInVehicle;

    this.handleSettingsChange();
  }

  setPlaytestVehicleName(name: string) {
    this.settings.playtestVehicleName = name;

    this.handleSettingsChange();
  }

  private handleSettingsChange = () => {
    invokeWEApi(WEApi.SettingsSet, this.settings);
    this.settingsVault.set(this.settings);
  };
};
