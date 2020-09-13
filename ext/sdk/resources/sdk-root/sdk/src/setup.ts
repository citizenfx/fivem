import { state } from './api/api';
import { States } from './api/api.types';
import * as serverInstaller from './serverInstaller';

export async function run() {
  state.toState(States.preparing);

  await (async () => {
    let total = 0;
    let downloaded = 0;
    const setContentLength = (length) => total = length;
    const setDataLength = (length) => {
      downloaded += length;

      state.ackFxserverDownloadState(total, downloaded);
    };

    await serverInstaller.downloadLatestServer(setContentLength, setDataLength);
  })();

  await (async () => {
    let total = 0;
    let unpacked = 0;
    const setContentLength = (length) => total = length;
    const setDataLength = (length) => {
      unpacked += length;

      state.ackFxserverUnpackState(total, unpacked);
    };

    await serverInstaller.unpackLatestServer(setContentLength, setDataLength);
  })();
}
