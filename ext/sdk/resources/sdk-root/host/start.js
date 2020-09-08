const fs = require('fs');
const os = require('os');
const path = require('path');
const concurrently = require('concurrently');
const { argv } = require('yargs');

const paths = require('./paths');
const env = require('dotenv').parse(fs.readFileSync('./shell/.env'));

const hostname = argv.hostname || os.hostname();
const port = parseInt(argv.port || env.PORT || '3000', 10);

function rebuild() {
  console.log('Rebuilding theia native node modules');

  // from https://github.com/eclipse-theia/theia/blob/master/dev-packages/application-manager/src/rebuild.ts#L23
  const nativeModules = ['@theia/node-pty', 'nsfw', 'native-keymap', 'find-git-repositories', 'drivelist'];

  return nativeModules.reduce((acc, nativeModule) => acc.then(() => concurrently([{
    name: `personality-theia:rebuild(${nativeModule})`,
    command: `yarn --cwd ${path.join(paths.personalityTheia, 'node_modules', nativeModule)} run install`,
  }])), Promise.resolve());
}

function prebuild() {
  const personalityTheiaFxdkAppLibPath = path.join(paths.personalityTheiaBrowserApp, 'lib');

  try {
    fs.statSync(personalityTheiaFxdkAppLibPath);

    console.log('No need for theia app prebuild, skipping');

    return Promise.resolve();
  } catch (e) {
    console.log('Prebuilding theia app');

    return concurrently([
      { name: 'theia:browser-app:build', command: `yarn --cwd ${paths.personalityTheiaBrowserApp} build` },
    ]);
  }
}

function start() {
  return concurrently([
    { name: 'theia:fxdk-game-view:watch', command: `yarn --cwd ${paths.personalityTheiaFxdkGameView} watch` },
    { name: 'theia:browser-app:watch', command: `yarn --cwd ${paths.personalityTheiaBrowserApp} watch` },
    {
      name: 'shell:start',
      command: `yarn cross-env PORT=${port} HOST=${hostname} yarn --cwd ${paths.shell} start`,
    },
    {
      name: 'theia:browser-app:start',
      command: `yarn --cwd ${paths.personalityTheiaBrowserApp} start --port=${port + 1} --hostname=${hostname}`,
    },
  ]);
}

Promise.resolve()
  .then(rebuild)
  .then(prebuild)
  .then(start)
  .catch(() => process.exit(1));
