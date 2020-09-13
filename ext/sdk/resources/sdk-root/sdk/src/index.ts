import './bootstrap';
import * as paths from './paths';
import { startShellApp } from './shell/shell';
import * as setup from './setup';
import { state } from './api/api';
import { States } from './api/api.types';

const sdkUrl = GetConvar('sdk_url');
const selfHosted = sdkUrl === 'http://localhost:35419/';


if (selfHosted) {
  try {
    process.chdir(paths.sdkRootTheia);
    const fakeArgv = [...process.argv, '--plugins=local-dir:plugins'];

    require('../host/personality-theia/server')(35420, 'localhost', fakeArgv);
  } catch (e) {
    console.log('personality-theia has failed to start');
    console.error(e);
  }
}

Promise.resolve()
  .then(() => startShellApp(paths.sdkRootShell))
  .then(() => setup.run())
  .then(() => state.toState(States.ready))
  .catch((e) => console.error(e));

emit('sdk:openBrowser', sdkUrl);

setTimeout(() => {
  emit('sdk:startGame');
}, 2500);
