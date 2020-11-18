import * as paths from './paths';
import * as config from './config';

export const launchTheia = async () => {
  if (config.selfHosted) {
    try {
      process.chdir(paths.sdkRootTheia);
      const fakeArgv = [...process.argv, '--plugins=local-dir:plugins'];

      return require('../../host/personality-theia/backend')(35420, 'localhost', fakeArgv);
    } catch (e) {
      console.log('personality-theia has failed to start');
      console.error(e);
    }
  }
};
