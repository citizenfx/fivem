import { CameraController } from "./camera-controller";

setTimeout(() => {
  SetOverrideWeather('EXTRASUNNY');
  SetWeatherTypeNow('EXTRASUNNY');
  SetWeatherTypePersist('EXTRASUNNY');

  NetworkOverrideClockTime(12, 0, 0);
  PauseClock(true);
}, 0);

setTick(() => {
  const camPos = CameraController.getPosition();

  SetRandomVehicleDensityMultiplierThisFrame(0);
  SetParkedVehicleDensityMultiplierThisFrame(0);

  SetPedDensityMultiplierThisFrame(0);
  SetVehicleDensityMultiplierThisFrame(0);

  SetGarbageTrucks(false);
  SetRandomBoats(false);

  HideHudComponentThisFrame(7);
  HideHudComponentThisFrame(9);
  HideHudAndRadarThisFrame();

  RemoveVehiclesFromGeneratorsInArea(
    camPos.x - 500, camPos.y - 500, camPos.z - 500,
    camPos.x + 500, camPos.y + 500, camPos.z + 500,
    0,
  );
});
