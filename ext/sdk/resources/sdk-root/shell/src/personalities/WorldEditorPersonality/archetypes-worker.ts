import { expose } from 'threads/worker';
import { Searcher } from 'fast-fuzzy';

const entryCollator = new Intl.Collator(undefined, {
  usage: 'sort',
});

function entriesSorter(a: string, b: string) {
  return entryCollator.compare(a, b);
};

let archetypes = [];
let archetypesReady = false;
let archetypesSearcher;

const archetypesLoadPromise = (async () => {
  try {
    const response = await fetch('http://localhost:35419/archetypes.json');

    if (response.status >= 400) {
      throw new Error('Archetypes unavailable');
    }

    archetypes = Object.values(await response.json()).flat().sort(entriesSorter);
    archetypesSearcher = new Searcher(archetypes);

    console.log('Hi form archetypes worker, archetypes are loaded');
  } catch (e) {
    console.log('Failed to fetch archetypes', e);
  } finally {
    archetypesReady = true;
  }
})();

expose({
  search(term: string) {
    return archetypesSearcher?.search(term) || [];
  },
  async getAll() {
    if (!archetypesReady) {
      await archetypesLoadPromise;
    }

    return archetypes;
  },
});
