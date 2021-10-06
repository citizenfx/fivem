const path = require('path');
const concurrently = require('concurrently');

const personalityFXCode = path.join(__dirname, '../../fxcode/fxdk');

function start() {
  return concurrently([
    { name: 'shell:server:watch', command: `yarn --cwd ../ watch:server` },
    { name: 'shell:client:watch', command: `yarn --cwd ../ watch:client` },
    { name: 'fxcode:watch', command: `yarn --cwd ${personalityFXCode} watch` },
  ]);
}

Promise.resolve()
  .then(start)
  .catch(() => process.exit(1));
