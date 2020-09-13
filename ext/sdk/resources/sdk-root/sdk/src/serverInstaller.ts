import * as mkdirp from 'mkdirp';
import * as fs from 'fs';
import * as path from 'path';
import * as http from 'https';
import * as yauzl from 'yauzl';
import * as rimrafSync from 'rimraf';
import { promisify } from 'util';
import * as paths from './paths';

const rimraf = promisify(rimrafSync);

const serversPath = paths.serverContainer;
const artifactsPath = paths.serverArtifacts;

try {
  fs.statSync(artifactsPath);
} catch (e) {
  mkdirp.sync(artifactsPath);
}

const artifacts = {
  '2972': 'https://runtime.fivem.net/artifacts/fivem/build_server_windows/master/2972-45c63bcfd6e87b0326131c5a266ac3c0c4623f16/server.zip',
};

function getArtifactPath(version: string): string {
  return path.join(artifactsPath, `${version}.zip`);
}

function getArtifactExtractionPath(version: string): string {
  return path.join(serversPath, version);
}

export type Setter = (size: number) => void;

export async function downloadServer(version: string, setContentLength: Setter, setDataLength: Setter) {
  const url = artifacts[version];

  const artifactPath = getArtifactPath(version);

  try {
    await fs.promises.stat(artifactPath);
    await fs.promises.unlink(artifactPath);
  } catch (e) { }

  return new Promise((resolve, reject) => {
    http.get(url, (res) => {
      const artifactWriteStream = fs.createWriteStream(artifactPath);

      const contentLength = parseInt(res.headers['content-length'] || '0', 10) || 0;
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

export async function unpackServer(version: string, setContentLength: Setter, setDataLength: Setter) {
  const artifactPath = getArtifactPath(version);
  const artifactExtractionPath = getArtifactExtractionPath(version);

  try {
    await fs.promises.stat(artifactExtractionPath);
    await rimraf(artifactExtractionPath);
  } catch (e) { }

  await mkdirp(artifactExtractionPath);

  return new Promise((resolveUnpack, rejectUnpack) => {
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

          setTimeout(tryStat, 100);
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

export async function downloadLatestServer(setContentLength: Setter, setDataLength: Setter) {
  const latestVersion = Object.keys(artifacts)[0];

  try {
    const artifactPath = getArtifactPath(latestVersion);

    await fs.promises.stat(artifactPath);

    return;
  } catch (e) {
    // That means we need to download artifact, but before we need to clear extraction site
    const serverPath = getArtifactExtractionPath(latestVersion);

    await rimraf(serverPath);
  }

  await downloadServer(latestVersion, setContentLength, setDataLength);
}

export async function prepareServer(version: string) {
  const serverPath = getArtifactExtractionPath(version);
  const componentsPath = path.join(serverPath, 'components.json');
  const componentsContent = await fs.promises.readFile(componentsPath);

  const components = JSON.parse(componentsContent.toString('utf8'));
  const newComponents = components.filter((component) => component !== 'svadhesive');

  await fs.promises.writeFile(componentsPath, JSON.stringify(newComponents, null, 2));
}

export async function unpackLatestServer(setContentLength: Setter, setDataLength: Setter) {
  const latestVersion = Object.keys(artifacts)[0];

  try {
    const serverPath = getArtifactExtractionPath(latestVersion);

    await fs.promises.stat(serverPath);

    return;
  } catch (e) {
    // This means we need to unpack and prepare :)
  }

  await unpackServer(latestVersion, setContentLength, setDataLength);
  await prepareServer(latestVersion);
}
