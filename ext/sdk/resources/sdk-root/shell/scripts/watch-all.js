const path = require('path');
const concurrently = require('concurrently');

const withFXCodeWatch = !process.argv.find(arg => arg.startsWith('--no-fxcode'));
const withGameWatch = !process.argv.find(arg => arg.startsWith('--no-game'));

const personalityFXCode = path.join(__dirname, '../../fxcode/fxdk');
const game = path.join(__dirname, '../../../sdk-game');

function start() {
  const tasks = [
    { name: 'shell:server:watch', command: `yarn --cwd ../ watch:server` },
    { name: 'shell:client:watch', command: `yarn --cwd ../ watch:client` },
  ];

  if (withGameWatch) {
    tasks.push({ name: 'game:watch', command: `yarn --cwd ${game} start` });
  }

  if (withFXCodeWatch) {
    tasks.push({ name: 'fxcode:watch', command: `yarn --cwd ${personalityFXCode} watch` });
  }

  return concurrently(tasks);
}

Promise.resolve()
  .then(start)
  .catch(() => process.exit(1));
