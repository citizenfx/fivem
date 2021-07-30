import './environment-manager';
import { CameraManager } from './camera-manager';
import { MapManager } from './map-manager';
import { PreviewManager } from './preview-manager';
import { SettingsManager } from './settings-manager';
import { Playtest } from './playtest';

SettingsManager.preinit();
CameraManager.preinit();
MapManager.preinit();
Playtest.preinit();

setTimeout(() => {
  CameraManager.init();
  MapManager.init();

  setTick(() => {
    CameraManager.update();
    PreviewManager.update();
    MapManager.update();
    Playtest.update();
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
