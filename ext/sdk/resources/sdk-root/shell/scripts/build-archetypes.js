const fs = require('fs');
const xml = require('fast-xml-parser');
const chalk = require('chalk');
const path = require('path');
const yargs = require('yargs/yargs');

const entryCollator = new Intl.Collator(undefined, {
  usage: 'sort',
});

const { argv } = yargs(process.argv.slice(2));

const verbose = argv.verbose;

const srcPath = argv.path || 'C:\\dev\\fivem\\ytyps';
const dstPath = path.join(__dirname, '../src/personalities/WorldEditorPersonality/WorldEditorToolbar/ObjectsBrowser/worker');

function panic(...args) {
  console.error(chalk.black.bgRed(...args));
  process.exit(1);
}

try {
  const stat = fs.statSync(srcPath);

  if (!stat.isDirectory()) {
    panic('Source path', srcPath, 'is not a directory');
  }

  console.log(chalk`Source path: {yellow ${srcPath}}`);
} catch (e) {
  panic('Could not find source path at', srcPath);
}

console.log(chalk`Destination path: {yellow ${dstPath}}`);

const sourceFiles = fs.readdirSync(srcPath);

let assets = [];

console.log('Num source files:', sourceFiles.length);

(async () => {
  let done = 0;

  await Promise.all(sourceFiles.map(async (srcFile) => {
    const items = await parseFile(srcFile);
    done++;

    if (items) {
      console.log(`${((done / sourceFiles.length) * 100)|0}%\t`, srcFile, '#items:', items.length);

      assets = assets.concat(items);
    }
  }));

  console.log('#asset:', assets.length);

  assets.sort((a, b) => entryCollator.compare(a, b));

  const archetypesStream = fs.createWriteStream(path.join(dstPath, 'archetypes.ts'), {
    flags: 'w',
  });

  archetypesStream.write('export const archetypes = ');
  archetypesStream.write(JSON.stringify(assets));
  archetypesStream.write(';\n');

  await new Promise((resolve) => archetypesStream.end(resolve));
})();

async function parseFile(file) {
  const content = (await fs.promises.readFile(path.join(srcPath, file))).toString();

  try {
    const json = xml.parse(content, {
      attributeNamePrefix: '@',
      ignoreAttributes: false,
    });

    const archetypes = get(json, 'CMapTypes', 'archetypes', 'Item');
    if (!archetypes) {
      return archetypes;
    }



    return archetypes
      .filter((item) => {
        if (item['@type'] !== 'CBaseArchetypeDef') {
          return false;
        }

        if (typeof item.assetType !== 'string') {
          return false;
        }

        if (item.assetType.indexOf('ASSET_TYPE_') !== 0) {
          return false;
        }

        return true;
      })
      .map((item) => item.name);
  } catch (e) {
    console.error(chalk.red('Failed to parse', file, e.message));
    verbose && console.error(e);
    return null;
  }
}

function get(obj, ...args) {
  let cobj = obj;

  for (const arg of args) {
    if (cobj[arg]) {
      cobj = cobj[arg];
    } else {
      return undefined;
    }
  }

  return cobj;
}

function parseFlags(s) {
  const f = parseInt(s, 10).toString(2).split('').reverse();

  return f.reduce((acc, n, i) => {
    if (n === '0') {
      return acc;
    }

    acc.push(`unk${i+1}`);

    return acc;
  }, []);
}
