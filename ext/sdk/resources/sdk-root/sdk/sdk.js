Error.prepareStackTrace = null;
const path = require('path');

const shellServer = require('./shellServer');
const setup = require('./setup');

const sdkUrl = GetConvar('sdk_url');
const selfHosted = sdkUrl === 'http://localhost:35419/';


const hostPath = path.join(process.cwd(), 'citizen/sdk/sdk-root/host');
const personalityTheiaPath = path.join(hostPath, 'personality-theia');
const shellPath = path.join(hostPath, 'shell');

if (selfHosted) {
  try {
    process.chdir(personalityTheiaPath);
    require('../host/personality-theia/server')(35420, 'localhost');
  } catch (e) {
    console.log('personality-theia has failed to start');
    console.error(e);
  }
}

Promise.resolve()
  .then(() => shellServer.start(shellPath))
  .then(() => setup.run())
  .then(() => shellServer.sendClientEvent({ type: 'state:transition', data: 'ready' }))
  .catch((e) => console.error(e));

emit('sdk:openBrowser', sdkUrl);

setTimeout(() => {
	emit('sdk:startGame');
}, 2500);
