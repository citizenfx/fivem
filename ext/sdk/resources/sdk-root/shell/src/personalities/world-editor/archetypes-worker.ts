import { expose } from 'threads/worker';
import fuzzysort from 'fuzzysort';
import { Archetypes } from 'backend/world-editor/world-editor-archetypes-types';
import { ARCHETYPE_FILE_INDEX, ARCHETYPE_LOD_DIST } from 'backend/world-editor/world-editor-archetypes-constants';
import { apiHost } from 'utils/apiHost';

// @ts-expect-error check for worker runtime
if (typeof WorkerGlobalScope !== 'undefined' && self instanceof WorkerGlobalScope) {
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

  let archetypesData: Archetypes;
  let archetypes: string[] = [];
  let archetypesReady = false;
  let archetypesPrepared;

  const archetypesLoadPromise = (async () => {
    try {
      const response = await fetch(`http://${apiHost.host}:${apiHost.port}/archetypes.json`);

      if (response.status >= 400) {
        throw new Error('Archetypes unavailable');
      }

      archetypesData = await response.json();

      archetypes = Object.keys(archetypesData.archetypes).sort(entriesSorter) as string[];
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
    async getArchetypeFileName(archetypeName: string) {
      if (!archetypesReady) {
        await archetypesLoadPromise;
      }

      return archetypesData.files[archetypesData.archetypes[archetypeName]?.[ARCHETYPE_FILE_INDEX]];
    },
    async getArchetypeLodDist(archetypeName: string) {
      if (!archetypesReady) {
        await archetypesLoadPromise;
      }

      return archetypesData.archetypes[archetypeName]?.[ARCHETYPE_LOD_DIST];
    },
    async getAll() {
      if (!archetypesReady) {
        await archetypesLoadPromise;
      }

      return archetypes;
    },
  });
}
