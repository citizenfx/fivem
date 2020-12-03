const fs = require('fs');
const os = require('os');
const path = require('path');
const concurrently = require('concurrently');
const { argv } = require('yargs');

const env = require('dotenv').parse(fs.readFileSync(path.join(__dirname, '../.env')));

const hostname = argv.hostname || os.hostname();
const port = parseInt(argv.port || env.PORT || '3000', 10);

const personalityTheia = path.join(__dirname, '../../personality-theia');
const personalityTheiaApp = path.join(personalityTheia, 'fxdk-app');
const personalityTheiaGameView = path.join(personalityTheia, 'fxdk-game-view');
const personalityTheiaProject = path.join(personalityTheia, 'fxdk-project');

function rebuild() {
  if (argv.norebuild) {
    return;
  }

  console.log('Rebuilding theia native node modules');

  // from https://github.com/eclipse-theia/theia/blob/master/dev-packages/application-manager/src/rebuild.ts#L23
  const nativeModules = ['@theia/node-pty', 'nsfw', 'native-keymap', 'find-git-repositories', 'drivelist'];

  return nativeModules.reduce((acc, nativeModule) => acc.then(() => concurrently([{
    name: `personality-theia:rebuild(${nativeModule})`,
    command: `yarn --cwd ${path.join(personalityTheia, 'node_modules', nativeModule)} run install`,
  }])), Promise.resolve());
}

function prebuild() {
  const personalityTheiaFxdkAppLibPath = path.join(personalityTheiaApp, 'lib');

  try {
    fs.statSync(personalityTheiaFxdkAppLibPath);

    console.log('No need for theia app prebuild, skipping');

    return Promise.resolve();
  } catch (e) {
    console.log('Prebuilding theia app');

    return concurrently([
      { name: 'theia:browser-app:build', command: `yarn --cwd ${personalityTheiaApp} build` },
    ]);
  }
}

function start() {
  return concurrently([
    { name: 'theia:fxdk-game-view:watch', command: `yarn --cwd ${personalityTheiaGameView} watch` },
    { name: 'theia:fxdk-project:watch', command: `yarn --cwd ${personalityTheiaProject} watch` },
    { name: 'theia:fxdk-app:watch', command: `yarn --cwd ${personalityTheiaApp} watch` },
    { name: 'shell:server:watch', command: `yarn --cwd ../ watch:server` },
    {
      name: 'shell:client:start',
      command: `yarn cross-env PORT=${port} HOST=${hostname} yarn --cwd ../ start:client`,
    },
    {
      name: 'theia:fxdk-app:start',
      command: `yarn --cwd ${personalityTheiaApp} start --port=${port + 1} --hostname=${hostname}`,
    },
  ]);
}

Promise.resolve()
  .then(rebuild)
  .then(prebuild)
  .then(start)
  .catch(() => process.exit(1));
