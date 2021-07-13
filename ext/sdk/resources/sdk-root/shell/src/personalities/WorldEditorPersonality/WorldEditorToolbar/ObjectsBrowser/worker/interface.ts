import { autorun, makeAutoObservable } from 'mobx';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { GameState } from 'store/GameState';
import { spawn, Worker, Pool } from 'threads';

export const ArchetypesStore = new class {
  public archetypes: string[] = [];

  constructor() {
    makeAutoObservable(this);
  }

  set(archetypes: string[]) {
    this.archetypes = archetypes;
  }
};

function spawnPool() {
  return Pool(() => spawn(new Worker('./index.ts')), 3);
}

let pool = spawnPool();

export async function searchArchetypes(term: string): Promise<string[]> {
  return pool.queue((searcher) => searcher.search(term));
}

export async function fetchAllArchetypes(): Promise<void> {
  if (ArchetypesStore.archetypes.length === 0) {
    ArchetypesStore.set(await pool.queue((searcher) => searcher.getAll()));
  }
}

export async function refreshArchetypesCollection() {
  pool.terminate(true);
  pool = spawnPool();

  fetchAllArchetypes();
}

// Watching for collection changes
(() => {
  let wasReady;

  autorun(() => {
    const weReady = WorldEditorState.ready;
    const archetypesReady = GameState.archetypesCollectionReady

    if (wasReady === undefined) {
      wasReady = archetypesReady;
      return;
    }

    if (weReady && archetypesReady && !wasReady) {
      refreshArchetypesCollection();
    }

    wasReady = archetypesReady;
  });
})();
