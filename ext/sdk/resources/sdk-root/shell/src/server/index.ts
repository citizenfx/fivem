(Error as any).prepareStackTrace = null;

process.on('uncaughtException', (error) => {
  console.log('UNHANDLED EXCEPTION', error);
  process.exit(-1);
});

process.on('unhandledRejection', (error) => {
  console.log('UNHANDLED REJECTION', error);
  process.exit(-1);
});

import * as config from './config';
import { startShellApp } from './shell';
import { state } from './api/api';
import { States } from 'shared/api.types';
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
