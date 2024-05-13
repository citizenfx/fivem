import { DEFAULT_SERVER_LOCALE } from 'cfx/base/serverUtils';

import { IAutocompleteIndex } from './types';
import { IServerView } from '../types';

export class AutocompleteIndexer {
  private tags: Record<string, number> = {};

  private locales: Record<string, number> = {};

  add(server: IServerView) {
    if (server.locale !== DEFAULT_SERVER_LOCALE) {
      this.locales[server.locale] = (this.locales[server.locale] || 0) + 1;
    }

    if (server.tags) {
      for (const tag of server.tags) {
        const lctag = tag.toLowerCase();

        this.tags[lctag] = (this.tags[lctag] || 0) + 1;
      }
    }
  }

  reset() {
    this.tags = {};
    this.locales = {};
  }

  getIndex(): IAutocompleteIndex {
    const tagSequence = enlist(this.tags).slice(0, 50);
    const localesSequence = enlist(this.locales);

    return {
      tag: {
        sequence: tagSequence,
        items: tagSequence.reduce((acc, tag) => {
          acc[tag] = {
            count: this.tags[tag] || 0,
          };

          return acc;
        }, {}),
      },

      locale: {
        sequence: localesSequence,
        items: localesSequence.reduce((acc, locale) => {
          acc[locale] = {
            count: this.locales[locale] || 0,
            locale,
            country: locale.split('-')[1] || locale,
          };

          return acc;
        }, {}),
      },
    };
  }
}

function enlist(object: Record<string, number>): string[] {
  // Most popular on top
  return Object.entries(object)
    .filter(([, count]) => count > 1)
    .sort(([, countA], [, countB]) => countB - countA)
    .map(([label]) => label);
}
