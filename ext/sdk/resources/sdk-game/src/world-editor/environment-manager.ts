import { CameraManager } from "./camera-manager";
import { WESetEnvirnomentType } from "@sdk-root/backend/world-editor/world-editor-types";
import { invokeWEApi, onWEApi } from "./utils";
import { WEApi } from "@sdk-root/backend/world-editor/world-editor-game-api";

const int2uint = (() => {
  const ab = new ArrayBuffer(4);
  const int = new Int32Array(ab);
  const uint = new Uint32Array(ab);

  return (n: number): number => {
    int[0] = n;

    return uint[0];
  };
})();

onWEApi(WEApi.EnvironmentSet, (request) => {
  switch (request.type) {
    case WESetEnvirnomentType.TIME: {
      NetworkOverrideClockTime(request.hours, request.minutes, 0);
      ackEnvironment();
      break;
    }

    case WESetEnvirnomentType.RANDOM_WEATHER: {
      ClearWeatherTypePersist();
      ClearOverrideWeather();
      SetRandomWeatherType();
      ackEnvironment();
      break;
    }

    case WESetEnvirnomentType.PERSISTENT_WEATHER: {
      SetOverrideWeather(request.weather);
      SetWeatherTypeNow(request.weather);
      SetWeatherTypePersist(request.weather);
      ackEnvironment();
      break;
    }
  }
});

setTimeout(() => {
  SetOverrideWeather('EXTRASUNNY');
  SetWeatherTypeNow('EXTRASUNNY');
  SetWeatherTypePersist('EXTRASUNNY');
}, 0);

setInterval(ackEnvironment, 500);

setTick(() => {
  const camPos = CameraManager.getPosition();

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

function ackEnvironment() {
  invokeWEApi(WEApi.EnvironmentAck, {
    hours: GetClockHours(),
    minutes: GetClockMinutes(),
    prevWeather: int2uint(GetPrevWeatherTypeHashName()),
    nextWeather: int2uint(GetPrevWeatherTypeHashName()),
  });
}
