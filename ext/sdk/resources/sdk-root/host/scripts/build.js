const paths = require('../paths');

const concurrently = require('concurrently');

function rebuild() {
  console.log('Rebuilding theia native node modules');

  // from https://github.com/eclipse-theia/theia/blob/master/dev-packages/application-manager/src/rebuild.ts#L23
  const nativeModules = ['@theia/node-pty', 'nsfw', 'native-keymap', 'find-git-repositories', 'drivelist'];

  return nativeModules.reduce((acc, nativeModule) => acc.then(() => concurrently([{
    name: `personality-theia:rebuild(${nativeModule})`,
    command: `yarn --cwd ${paths.personalityTheia} electron-rebuild -f -m node_modules/${nativeModule}`,
  }])), Promise.resolve());
}

function build() {
  return concurrently([
    { name: 'shell:build', command: `yarn --cwd ${paths.shell} build` },
    { name: 'personality-theia:build', command: `yarn --cwd ${paths.personalityTheia} build` },
  ]);
}

Promise.resolve()
  .then(rebuild)
  .then(build)
  .catch((e) => process.exit(1));
