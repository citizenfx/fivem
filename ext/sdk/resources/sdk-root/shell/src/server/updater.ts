import * as fs from 'fs';
import * as tar from 'tar-fs';
import * as path from 'path';
import * as paths from './paths';
import * as config from './config';
import { state } from './api/api';
import { States } from 'shared/api.types';
import { setStatus } from './api/StatusesApi';
import { rimraf } from './rimraf';
import { createDeferred } from '../shared/utils';
import { updaterStatuses } from 'shared/api.statuses';

export interface UpdaterData {
  personality_theia_tar_mtimeMs: number,
}

export const updaterPass = async () => {
  state.toState(States.preparing);

  await maybeUnpackTheia();
};

async function maybeUnpackTheia(): Promise<void> {
  // if not self-hosted - skip
  if (!config.selfHosted) {
    return;
  }

  // No archive - no unpack
  const archiveStats = await statSafe(paths.sdkRootTheiaArchive);
  if (!archiveStats) {
    return;
  }

  // archive contains root `build` dir, so when unpacked it will be this path
  const extractedArchivePath = path.join(paths.sdkRoot, 'build');

  const archiveSize = archiveStats.size;
  let unpackedSize = 0;
  let currentFileName = '';

  const sendStatus = () => setStatus(updaterStatuses.state, { completed: unpackedSize / archiveSize, currentFileName });

  // rm theia dir
  if (await statSafe(paths.sdkRootTheia)) {
    currentFileName = 'Removing old install...';
    sendStatus();

    await rimraf(paths.sdkRootTheia);
  }

  // rm maybe-present build dir, might be interrupted earlier unpack attempt
  if (await statSafe(extractedArchivePath)) {
    await rimraf(extractedArchivePath);
  }

  const unpackDefer = createDeferred();

  const archiveReadStream = fs.createReadStream(paths.sdkRootTheiaArchive);
  const extractorStream = tar.extract(paths.sdkRoot, {
    mapStream(fileReadStream, headers) {
      currentFileName = `Unpacking ${headers.name}...`;

      fileReadStream.on('data', (data) => {
        unpackedSize += data.length;

        sendStatus();
      });

      return fileReadStream;
    },
  });

  extractorStream.on('finish', () => unpackDefer.resolve());

  archiveReadStream.pipe(extractorStream);

  // Waiting for unpack to finish
  await unpackDefer.promise;

  currentFileName = 'Finishing...';
  sendStatus();

  // Deleting archive and renaming `build` to `personality-theia`
  await Promise.all([
    fs.promises.unlink(paths.sdkRootTheiaArchive),
    fs.promises.rename(extractedArchivePath, paths.sdkRootTheia),
  ]);
}

async function statSafe(entryPath: string): Promise<fs.Stats | null> {
  try {
    return await fs.promises.stat(entryPath);
  } catch (e) {
    return null;
  }
}
