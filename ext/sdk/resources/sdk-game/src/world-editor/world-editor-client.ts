import { sendSdkMessage } from '../client/sendSdkMessage';
import { CameraController } from './camera-controller';
import './environment-manager';
import { PreviewManager } from './preview-manager';
import { SelectionController } from './selection-controller';

setTimeout(() => {
  SetPlayerControl(PlayerId(), false, 0);
  CameraController.init();

  setTick(() => {
    CameraController.update();
    SelectionController.update();
    PreviewManager.update();
  });

  ShutdownLoadingScreen();
  DoScreenFadeIn(0);

  sendSdkMessage('we:ready');
}, 0);

on('disconnecting', () => {
  CameraController.destroy();
});

on('onResourceStop', (resourceName: string) => {
  if (resourceName === GetCurrentResourceName()) {
    CameraController.destroy();
  }
});
