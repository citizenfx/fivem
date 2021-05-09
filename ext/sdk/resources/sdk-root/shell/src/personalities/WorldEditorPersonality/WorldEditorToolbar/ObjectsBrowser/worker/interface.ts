import { spawn, Worker, Pool } from 'threads';

const pool = Pool(() => spawn(new Worker('./index.ts')), 3);

let allArchetypes: string[];

export async function searchArchetypes(term: string): Promise<string[]> {
  return pool.queue((searcher) => searcher.search(term));
}

export async function getAllArchetypes(): Promise<string[]> {
  if (!allArchetypes) {
    allArchetypes = await pool.queue((searcher) => searcher.getAll());
  }

  return allArchetypes;
}
