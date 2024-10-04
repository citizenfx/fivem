const fs = require('fs');
const path = require('path');

const localesPath = path.join(__dirname, '../../src/assets/languages');

module.exports = function (source) {
  const doneCallback = this.async();

  // Add whole locales dir as deps so when anything changes there we can recompile
  this.addContextDependency(localesPath);

  compileLocales(source, doneCallback);
};

async function compileLocales(source, doneCallback) {
  const localeFiles = await fs.promises.readdir(localesPath);

  const pairs = localeFiles.filter((file) => file.indexOf('locale-') === 0).map((file) => {
    const locale = path.basename(file, '.json').substring('locale-'.length).replace('_', '-').toLowerCase();
    if (!locale) {
      return;
    }

    return [locale, path.join(localesPath, file)];
  });

  const locales = [];

  for (const [locale, filepath] of pairs) {
    try {
      const content = JSON.parse((await fs.promises.readFile(filepath)).toString());

      locales.push(`'${locale}': ${compileTranslatedLines(content)}`);
    } catch (e) {
      console.error(e);

      return doneCallback(e);
    }
  }

  doneCallback(null, source.replace('/*@@EXPORTS@@*/', locales.join(', ')));
}

function compileTranslatedLines(lines) {
  return '{' + Object.entries(lines).map(([key, value]) => {
    return `'${key}': ${compileTranslatedLine(value)}`;
  }).join(', ') + '}';
}

const substitutesRegExp = /({{[\s]{0,}(?<word>.*?)[\s]{0,}}})/g;

/**
 * Compiles translated string into function
 *
 * For example:
 *  "Welcome to {{ gameBrand }}"  ->  ({gameBrand=""}={}) => `Welcome to ${gameBrand}`
 *
 * @param {string} line
 * @returns string
 */
function compileTranslatedLine(line) {
  // No substitutions - simple string
  if (line.indexOf('{{') === -1) {
    return `() => ${JSON.stringify(line)}`;
  }

  const wordsSet = new Set();

  const substitutes = [...line.matchAll(substitutesRegExp)].map((match) => {
    wordsSet.add(match.groups.word);

    return {
      placeholder: match[0],
      word: match.groups.word,
    };
  });

  let body = line;

  for (const substitute of substitutes) {
    body = body.replace(substitute.placeholder, '${' + substitute.word + '}');
  }

  const args = [...wordsSet].map((word) => `${word}=""`).join(`,`);

  return `({${args}}={}) => \`${body}\``;
}
