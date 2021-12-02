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

let lastSetTime: [number, number, number] = [0, 0, 0];
let freezeTime = false;

onWEApi(WEApi.EnvironmentSet, (request) => {
  switch (request.type) {
    case WESetEnvirnomentType.TIME: {
      lastSetTime[0] = request.hours;
      lastSetTime[1] = request.minutes;
      NetworkOverrideClockTime(request.hours, request.minutes, 0);
      ackEnvironment();
      break;
    }

    case WESetEnvirnomentType.FREEZE_TIME: {
      freezeTime = request.freeze;
      ackEnvironment
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
  invokeWEApi(WEApi.EnvironmentRequest, undefined);

  SetOverrideWeather('EXTRASUNNY');
  SetWeatherTypeNow('EXTRASUNNY');
  SetWeatherTypePersist('EXTRASUNNY');
}, 0);

setInterval(ackEnvironment, 500);

setTick(() => {
  if (freezeTime) {
    NetworkOverrideClockTime(...lastSetTime);
  }

  const camPos = CameraManager.getPosition();

  SetRandomVehicleDensityMultiplierThisFrame(0);
  SetParkedVehicleDensityMultiplierThisFrame(0);

  SetPedDensityMultiplierThisFrame(0);
  SetVehicleDensityMultiplierThisFrame(0);
  SetScenarioPedDensityMultiplierThisFrame(0, 0);

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
  if (!freezeTime) {
    lastSetTime[0] = GetClockHours();
    lastSetTime[1] = GetClockMinutes();
  }

  invokeWEApi(WEApi.EnvironmentAck, {
    hours: lastSetTime[0],
    minutes: lastSetTime[1],
    prevWeather: int2uint(GetPrevWeatherTypeHashName()),
    nextWeather: int2uint(GetPrevWeatherTypeHashName()),
  });
}
