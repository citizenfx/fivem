import { expose } from 'threads/worker';
import fuzzysort from 'fuzzysort';

const entryCollator = new Intl.Collator(undefined, {
  usage: 'sort',
});

const fuzzysortOptions: Fuzzysort.Options = {
  limit: 100,
  threshold: -10000,
};

function entriesSorter(a: string, b: string) {
  return entryCollator.compare(a, b);
};

let archetypes: string[] = [];
let archetypesReady = false;
let archetypesPrepared;

const archetypesLoadPromise = (async () => {
  try {
    const response = await fetch('http://localhost:35419/archetypes.json');

    if (response.status >= 400) {
      throw new Error('Archetypes unavailable');
    }

    archetypes = Object.values(await response.json()).flat().sort(entriesSorter) as string[];
    archetypesPrepared = archetypes.map((name) => fuzzysort.prepare(name));

    console.log('Hi form archetypes worker, archetypes are loaded');
  } catch (e) {
    console.log('Failed to fetch archetypes', e);
  } finally {
    archetypesReady = true;
  }
})();

expose({
  search(term: string) {
    if (!archetypesPrepared) {
      return [];
    }

    return fuzzysort.go(term, archetypesPrepared, fuzzysortOptions).map(({ target }) => target);
  },
  async getAll() {
    if (!archetypesReady) {
      await archetypesLoadPromise;
    }

    return archetypes;
  },
});
