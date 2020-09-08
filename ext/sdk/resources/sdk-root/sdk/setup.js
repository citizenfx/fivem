const mkdirp = require('mkdirp');
const path = require('path');
const fs = require('fs');

const shellServer = require('./shellServer');
const serverManager = require('./serverManager');

const localAppData = path.join(process.env.LOCALAPPDATA, 'citizenfx');
const setupFilePath = path.join(localAppData, 'setup.json');

function run() {
  try {
    fs.statSync(setupFilePath);

    return Promise.resolve();
  } catch (e) {
    mkdirp.sync(localAppData);

    fs.writeFileSync(setupFilePath, JSON.stringify(Date.now()));
  }

  return Promise.resolve()
    .then(() => {
      shellServer.sendClientEvent({ type: 'state:transition', data: 'preparing' });

      let total = 0;
      let downloaded = 0;
      const setContentLength = (length) => total = length;
      const setDataLength = (length) => {
        downloaded += length;

        shellServer.sendClientEvent({ type: 'state:fxserverDownload', data: { total, downloaded } });
      };

      return serverManager.downloadLatestServer(setContentLength, setDataLength);
    })
    .then(() => {
      let total = 0;
      let unpacked = 0;
      const setContentLength = (length) => total = length;
      const setDataLength = (length) => {
        unpacked += length;

        shellServer.sendClientEvent({ type: 'state:fxserverUnpack', data: { total, downloaded: unpacked } });
      };

      return serverManager.unpackLatestServer(setContentLength, setDataLength);
    })
    .then(serverManager.prepareLatestServer);
}

module.exports = {
  run,
};
