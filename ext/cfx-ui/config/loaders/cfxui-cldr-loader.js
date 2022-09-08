const cldrLanguages = require('cldr-localenames-modern/main/en/languages.json');
const cldrLanguageData = require('cldr-core/supplemental/languageData.json');
const cldrTerritoryInfo = require('cldr-core/supplemental/territoryInfo.json');
const cldrTerritories = require('cldr-localenames-modern/main/en/territories.json');

const cldrLocales = require('cldr-localenames-modern/main/en/localeDisplayNames.json');
const cldrSubTags = require('cldr-core/supplemental/likelySubtags.json');

const cldrjs = require('cldrjs');

cldrjs.load(cldrLocales, cldrLanguages, cldrTerritories, cldrSubTags);

const cldr = new cldrjs('en');

const validLocales = [...new Set([
  ...Object.entries(cldrTerritoryInfo.supplemental.territoryInfo).map(([k]) => k.toLowerCase()),
	...Object.entries(cldrLanguageData.supplemental.languageData).map(([k]) => k.toLowerCase()),
])];

const languageMap = Object.fromEntries(
  Object.entries(cldrLanguages.main.en.localeDisplayNames.languages)
    .map(([a, b]) => [b.toLowerCase().replace(/ /g, '-'), a.split('-')[0].toLowerCase()]),
);

const countryMap = Object.fromEntries(
  Object.entries(cldrTerritories.main.en.localeDisplayNames.territories)
    .map(([a, b]) => [b.toLowerCase().replace(/ /g, '-'), a.split('-')[0].toLowerCase()]),
);

const localeDisplayNames = {};

module.exports = function (source) {
  const doneCallback = this.async();

  compileCldr(source, doneCallback);
};

async function compileCldr(source, doneCallback) {
  const exportParts = [
    `validLocales: new Set(${JSON.stringify(validLocales)})`,
    `languageMap: ${JSON.stringify(languageMap)}`,
    `countryMap: ${JSON.stringify(countryMap)}`,
    `localeDisplayNames: ${JSON.stringify(localeDisplayNames)}`,
  ].join(', ');

  doneCallback(null, source.replace('/*@@EXPORTS@@*/', exportParts));
}
