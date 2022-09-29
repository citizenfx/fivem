import { returnTrue } from "cfx/utils/functional";
import { IListableServerView } from "./types";
import { IServerListConfig } from "../lists/types";
import { ISearchTerm } from "cfx/base/searchTermsParser";

type IFilter = (server: IListableServerView) => boolean;

export function filterList(servers: Record<string, IListableServerView>, sortedList: string[], config: IServerListConfig): string[] {
  const filters: IFilter[] = [];

  compileTagsFilters(filters, config);
  compileLocaleFilters(filters, config);
  compileEmptyFullFilters(filters, config);
  compileOnlyPremiumFilters(filters, config);
  compileSearchTermsFilters(filters, config);

  return sortedList.filter((id) => {
    const server = servers[id];

    for (const filter of filters) {
      if (!filter(server)) {
        return false;
      }
    }

    return true;
  });
}

function compileLocaleFilters(filters: IFilter[], config: IServerListConfig) {
  const localeEntries = Object.entries(config.locales);

  if (localeEntries.length) {
    filters.push(
      (server) => localeEntries.some(([locale, enabled]) => enabled === (server.locale === locale)),
    );
  }
}

function compileTagsFilters(filters: IFilter[], config: IServerListConfig) {
  const tagEntries = Object.entries(config.tags);

  if (tagEntries.length) {
    filters.push(
      (server) => tagEntries.some(([tag, enabled]) => enabled === Boolean(server.tagsMap[tag])),
    );
  }
}

function compileOnlyPremiumFilters(filters: IFilter[], config: IServerListConfig) {
  if (config.onlyPremium) {
    filters.push(
      (server) => Boolean(server.premium),
    );
  }
}

function compileEmptyFullFilters(filters: IFilter[], config: IServerListConfig) {
  const { hideFull, hideEmpty } = config;

  if (hideFull) {
    filters.push(
      (server) => !server.isFull,
    );
  }

  if (hideEmpty) {
    filters.push(
      (server) => !server.isEmpty,
    );
  }
}

function compileTermValueMatcher(term: ISearchTerm): (against: string) => boolean {
  if (!term.regexp) {
    const lowerCaseValue = term.value.toLowerCase();

    return (against) => against.toLowerCase().includes(lowerCaseValue);
  }

  const valueRegExp = new RegExp(term.value, 'i');

  return (against) => valueRegExp.test(against);
}

function compileSearchTermsFilters(filters: IFilter[], config: IServerListConfig) {
  const {
    searchTextParsed: terms
  } = config;

  if (!terms.length) {
    return;
  }

  for (const term of terms) {
    const { type, value, invert } = term;

    const valueMatcher = compileTermValueMatcher(term);

    let filter: IFilter | void;

    switch (type) {
      case 'name': {
        let localeFilter: IFilter = returnTrue;

        if (term.matchLocale) {
          const matchWith = term.matchLocale.with;

          switch (term.matchLocale.at) {
            case 'start': {
              localeFilter = (server) => server.locale.startsWith(matchWith);
              break;
            }
            case 'end': {
              localeFilter = (server) => server.locale.endsWith(matchWith);
            }
          }
        }

        filter = (server) => localeFilter(server) && valueMatcher(server.searchableName);
        break;
      }

      case 'locale': {
        filter = (server) => {
          const idx = server.locale.indexOf(value);

          const starts = idx === 0;
          const ends = idx === (server.locale.length - 2);

          return starts || ends;
        };
        break;
      }

      case 'category': {
        const { category } = term;

        filter = (server) => {
          const categoryMatcher = server.categories[category] || server.categories[category + 's'];
          if (!categoryMatcher) {
            return false;
          }

          switch (categoryMatcher.type) {
            case 'string': return valueMatcher(categoryMatcher.against);
            case 'array': return categoryMatcher.against.some((categoryMatchee) => valueMatcher(categoryMatchee));
          }
        };
        break;
      }
    }

    if (filter) {
      filters.push(
        invert
          ? (server) => !filter!(server)
          : filter,
      );
    }
  }
}
