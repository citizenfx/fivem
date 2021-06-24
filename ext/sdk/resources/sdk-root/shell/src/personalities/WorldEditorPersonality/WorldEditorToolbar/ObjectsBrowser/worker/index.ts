import { expose } from 'threads/worker';
import { Searcher } from 'fast-fuzzy';
import { archetypes } from './archetypes';

const archetypesSearcher = new Searcher(archetypes);

console.log('Hi form archetypes searcher worker');

expose({
  search(term: string) {
    return archetypesSearcher.search(term);
  },
  getAll() {
    return archetypes;
  },
});
