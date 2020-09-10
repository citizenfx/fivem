const mkdirp = require('mkdirp');
const rimraf = require('rimraf');
const path = require('path');
const http = require('https');
const fs = require('fs');
const yauzl = require("yauzl");

const serversPath = path.join(process.env.LOCALAPPDATA, 'citizenfx/sdk-storage/server');
const artifactsPath = path.join(serversPath, 'artifacts');

try {
  fs.statSync(artifactsPath);
} catch (e) {
  mkdirp.sync(artifactsPath);
}

const artifacts = {
  '2941': 'https://runtime.fivem.net/artifacts/fivem/build_server_windows/master/2941-bc06cecf1053f6448024cef488d7d71365b1942b/server.zip',
};

function getArtifactPath(version) {
  return path.join(artifactsPath, `${version}.zip`);
}

function getArtifactExtractionPath(version) {
  return path.join(serversPath, version);
}

function downloadServer(version, setContentLength, setDataLength) {
  const url = artifacts[version];

  return new Promise((resolve, reject) => {
    const artifactPath = getArtifactPath(version);

    try {
      fs.statSync(artifactPath);
      fs.unlinkSync(artifactPath);
    } catch (e) { }

    http.get(url, (res) => {
      const artifactWriteStream = fs.createWriteStream(artifactPath);

      const contentLength = parseInt(res.headers['content-length'], 10) || 0;
      setContentLength(contentLength);

      res.pipe(artifactWriteStream)

      artifactWriteStream.on('finish', () => {
        console.log('Finish writing artifact', artifactPath);
        artifactWriteStream.close();
        resolve(artifactPath);
      });

      res.on('data', (data) => setDataLength(data.length));
      res.on('end', () => {
        console.log('Finish downloading artifact', url);
      });
    }).on('error', (e) => reject(e));
  });
}

function unpackServer(version, setContentLength, setDataLength) {
  return new Promise((resolveUnpack, rejectUnpack) => {
    const artifactPath = getArtifactPath(version);
    const artifactExtractionPath = getArtifactExtractionPath(version);

    try {
      fs.statSync(artifactExtractionPath);
      rimraf.sync(artifactExtractionPath);
    } catch (e) { }

    mkdirp.sync(artifactExtractionPath);

    const p = (resolve, reject) => {
      let tries = 0;

      const tryStat = () => {
        tries++;

        try {
          setContentLength(fs.statSync(artifactPath).size);
          console.log('Artifact size acquired');
          return resolve();
        } catch (e) {
          if (tries > 5) {
            return reject(e);
          }

          setTimeout(tryState, 100);
        }
      };

      tryStat();
    };

    (new Promise(p))
      .then(() => new Promise((resolve, reject) => {
        yauzl.open(artifactPath, { lazyEntries: true }, (err, zipFile) => {
          if (err) {
            return rejectUnpack(err);
          }

          zipFile.readEntry();
          zipFile.on('entry', (entry) => {
            const entryPath = path.join(artifactExtractionPath, entry.fileName);

            if (entry.fileName.endsWith('/')) {
              mkdirp.sync(entryPath);
              zipFile.readEntry();
            } else {
              zipFile.openReadStream(entry, (err, readStream) => {
                if (err) {
                  return rejectUnpack(err);
                }

                setDataLength(entry.compressedSize);

                readStream.on('end', () => zipFile.readEntry());
                readStream.pipe(fs.createWriteStream(entryPath));
              });
            }
          });
          zipFile.on('close', () => resolveUnpack());
        });
      }))
  });
}

function downloadLatestServer(setContentLength, setDataLength) {
  const latestVersion = Object.keys(artifacts)[0];

  return downloadServer(latestVersion, setContentLength, setDataLength);
}

function unpackLatestServer(setContentLength, setDataLength) {
  const latestVersion = Object.keys(artifacts)[0];

  return unpackServer(latestVersion, setContentLength, setDataLength);
}

function prepareLatestServer() {
  return new Promise((resolve, reject) => {
    const latestVersion = Object.keys(artifacts)[0];

    const serverPath = getArtifactExtractionPath(latestVersion);
    const componentsPath = path.join(serverPath, 'components.json');

    const components = JSON.parse(fs.readFileSync(componentsPath).toString('utf8'));
    const newComponents = components.filter((component) => component !== 'svadhesive');

    fs.writeFileSync(componentsPath, JSON.stringify(newComponents, null, 2));

    resolve();
  });
}

module.exports = {
  downloadLatestServer,
  unpackLatestServer,
  prepareLatestServer,
};
