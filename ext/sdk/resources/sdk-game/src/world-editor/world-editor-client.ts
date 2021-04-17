import { CameraController } from './camera-controller';
import './environment-manager';
import { SelectionController } from './selection-controller';

setTimeout(() => {
  SetPlayerControl(PlayerId(), false, 0);
  CameraController.init();

  setTick(() => {
    CameraController.update();
    SelectionController.update();
  });

  ShutdownLoadingScreen();
}, 0);

on('onResourceStop', (resourceName: string) => {
  if (resourceName === GetCurrentResourceName()) {
    CameraController.destroy();
  }
});
