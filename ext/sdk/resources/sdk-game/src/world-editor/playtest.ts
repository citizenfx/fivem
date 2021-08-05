import { CameraManager } from "./camera-manager";
import { MapManager } from "./map-manager";
import { WECam } from "./map-types";
import { SelectionController } from "./selection-controller";
import { SettingsManager } from "./settings-manager";

export const Playtest = new class Playtest {
  private savedCam: WECam | null = null;

  private vehiclePending = false;
  private vehicleHandle: number | undefined;
  private vehicleModel: string | undefined;

  preinit() {
    on('we:enterPlaytestMode', this.enable);
    on('we:exitPlaytestMode', this.disable);
  }

  readonly enable = () => {
    MapManager.disable();
    SelectionController.disable();
    SelectionController.setSelectedEntity(null);

    this.savedCam = CameraManager.getCam();

    const cam = CameraManager.getPosition();
    const ped = PlayerPedId();

    const [success, z] = GetGroundZFor_3dCoord(cam.x, cam.y, cam.z, 0);
    if (success) {
      SetEntityCoords(ped, cam.x, cam.y, z, true, false, false, false);
    } else {
      SetEntityCoords(ped, cam.x, cam.y, cam.z, true, false, false, false);
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
  };

  readonly disable = () => {
    MapManager.enable();
    SelectionController.enable();
    CameraManager.init(true, 200);

    if (this.savedCam) {
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
