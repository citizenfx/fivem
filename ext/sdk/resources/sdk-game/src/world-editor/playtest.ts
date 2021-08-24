import { CameraManager } from "./camera-manager";
import { MapManager } from "./map-manager";
import { WECam } from "@sdk-root/backend/world-editor/world-editor-types";
import { SelectionController } from "./selection-controller";
import { SettingsManager } from "./settings-manager";
import { onWEApi } from "./utils";
import { WEApi } from "@sdk-root/backend/world-editor/world-editor-game-api";

export const Playtest = new class Playtest {
  private savedCam: WECam | null = null;

  private vehiclePending = false;
  private vehicleHandle: number | undefined;
  private vehicleModel: string | undefined;

  preinit() {
    onWEApi(WEApi.EnterPlaytestMode, this.enable);
    onWEApi(WEApi.ExitPlaytestMode, this.disable);
  }

  readonly enable = () => {
    MapManager.disable();
    SelectionController.disable();

    this.savedCam = CameraManager.getCam();

    const cam = CameraManager.getPosition();
    const ped = PlayerPedId();

    const forward = CameraManager.getForwardVector();

    const x = cam.x + forward.x * 5;
    const y = cam.y + forward.y * 5;

    const [success, z] = GetGroundZFor_3dCoord(x, y, cam.z, false);
    if (success) {
      SetEntityCoords(ped, x, y, z, true, false, false, false);
    } else {
      SetEntityCoords(ped, x, y, cam.z, true, false, false, false);
    }

    CameraManager.destroy(true, 200, true, true);

    if (SettingsManager.settings.playtestSpawnInVehicle && SettingsManager.settings.playtestVehicleName) {
      const vehicleModel = SettingsManager.settings.playtestVehicleName;

      if (IsModelInCdimage(vehicleModel) && IsModelAVehicle(vehicleModel)) {
        this.vehicleModel = vehicleModel;
        this.vehiclePending = true;
        RequestModel(vehicleModel);
      }
    }

    SetEntityHeading(ped, this.savedCam[5]);
    SetGameplayCamRelativeHeading(0);
  };

  readonly disable = (relative: boolean) => {
    const gameplayCam: WECam = [
      ...GetGameplayCamCoord(),
      ...GetGameplayCamRot(2),
    ] as WECam;

    MapManager.enable();
    SelectionController.enable();
    CameraManager.init(true, 200);

    if (relative) {
      CameraManager.setCam(gameplayCam);
    } else if (this.savedCam) {
      CameraManager.setCam(this.savedCam);
    }

    if (this.vehicleHandle !== undefined) {
      DeleteVehicle(this.vehicleHandle);
    }
  };

  update() {
    if (this.vehiclePending) {
      if (HasModelLoaded(this.vehicleModel)) {
        this.vehiclePending = false;

        const pos = GetEntityCoords(PlayerPedId());
        const heading = GetEntityHeading(PlayerPedId());

        this.vehicleHandle = CreateVehicle(this.vehicleModel, pos[0], pos[1], pos[2], heading, false, false);

        SetVehicleNeedsToBeHotwired(this.vehicleHandle, false);
        SetVehicleEngineOn(this.vehicleHandle, true, true, false);

        SetPedIntoVehicle(PlayerPedId(), this.vehicleHandle, -1);

        SetVehicleRadioEnabled(this.vehicleHandle, false);
      }
    }
  }
};
