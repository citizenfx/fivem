import './environment-manager';
import { CameraManager } from './camera-manager';
import { MapManager } from './map-manager';
import { PreviewManager } from './preview-manager';

CameraManager.preinit();
MapManager.preinit();

setTimeout(() => {
  SetPlayerControl(PlayerId(), false, 0);

  CameraManager.init();
  MapManager.init();

  setTick(() => {
    CameraManager.update();
    PreviewManager.update();
    MapManager.update();
  });

  ShutdownLoadingScreen();
  DoScreenFadeIn(0);
}, 0);

on('disconnecting', () => {
  CameraManager.destroy();
  MapManager.destroy();
});

on('onResourceStop', (resourceName: string) => {
  if (resourceName === GetCurrentResourceName()) {
    CameraManager.destroy();
    MapManager.destroy();
  }
});
