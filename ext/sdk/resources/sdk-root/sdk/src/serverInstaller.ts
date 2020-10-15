import * as mkdirp from 'mkdirp';
import * as fs from 'fs';
import * as path from 'path';
import * as http from 'https';
import * as yauzl from 'yauzl';
import * as rimrafSync from 'rimraf';
import { promisify } from 'util';

const rimraf = promisify(rimrafSync);


export const versionFilename = '.fxserver-version';

export type Setter = (size: number) => void;

export async function downloadArtifact(url: string, artifactPath: string, setContentLength: Setter, setDataLength: Setter) {
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

export async function unpackArtifact(artifactPath: string, artifactExtractionPath: string, setContentLength: Setter, setDataLength: Setter) {
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

export async function prepareServer(artifactExtractionPath: string, version: string) {
  const componentsPath = path.join(artifactExtractionPath, 'components.json');
  const componentsContent = await fs.promises.readFile(componentsPath);

  const components = JSON.parse(componentsContent.toString('utf8'));
  const newComponents = components.filter((component) => component !== 'svadhesive');

  await fs.promises.writeFile(componentsPath, JSON.stringify(newComponents, null, 2));

  // Separate to make sure it will be added in the very end
  // because installer relies on this file
  await fs.promises.writeFile(path.join(artifactExtractionPath, versionFilename), version);
}

