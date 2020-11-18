import './bootstrap';
import * as config from './config';
import { startShellApp } from './shell';
import { state } from './api/api';
import { States } from './api/api.types';
import { setupWellKnownPaths } from './setupWellKnownPaths';
import { updaterPass } from './updater';
import { launchTheia } from './theiaLauncher';

setupWellKnownPaths();

Promise.resolve()
  .then(startShellApp)
  .then(updaterPass)
  .then(launchTheia)
  .then(() => state.toState(States.ready))
  .catch((e) => console.error(e));

emit('sdk:openBrowser', config.sdkUrl);

setTimeout(() => {
  emit('sdk:startGame');
}, 2500);
