import { makeAutoObservable, runInAction } from "mobx";
import { GameState } from "store/GameState";
import { spawn, Worker, Pool } from 'threads';

function spawnPool() {
  return Pool(() => spawn(new Worker('../archetypes-worker.ts')), 3);
}

let pool = spawnPool();

export const ArchetypesState = new class ArchetypesState {
  archetypes: string[] = [];

  constructor() {
    makeAutoObservable(this);

    GameState.onArchetypeCollectionReady(this.reload);
  }

  async search(term: string): Promise<string[]> {
    return pool.queue((searcher) => searcher.search(term));
  }

  async load() {
    if (!pool) {
      return;
    }

    await pool.queue((searcher) => searcher.getAll()).then((archetypes) => runInAction(() => {
      this.archetypes = archetypes;
    }));
  }

  private readonly reload = () => {
    pool.terminate(true);
    pool = spawnPool();

    this.load();
  }
}();
