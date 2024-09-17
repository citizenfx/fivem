import { noop } from '@cfx-dev/ui-components';
import { makeAutoObservable } from 'mobx';

import { ISearchTerm, parseSearchTerms2 } from 'cfx/base/searchTermsParser';
import { clone, isObject } from 'cfx/utils/object';

import {
  IPartialServerListConfig,
  IServerListConfig,
  ServerListSortDir as ServersListSortDir,
  ServersListSortBy,
} from './types';

namespace LS_KEYS {
  export const FiltersPrefix = 'sfilters:';
  export const TagsPrefix = 'stags:';
}

export const DEFAULT_SORT_BY = ServersListSortBy.Boosts;
export const DEFAULT_SORT_DIR = ServersListSortDir.Desc;

export interface IServerListConfigControllerOptions {
  config: IPartialServerListConfig;

  /**
   * Callback to invoke when config changes
   */
  onChange?(config: IServerListConfig): void;

  /**
   * Whether or not config should be persisted in localStorage
   *
   * @default true
   */
  persitent?: boolean;
}

export class ServerListConfigController {
  private observableConfig: IServerListConfig;

  private sendableConfig: IServerListConfig;

  public readonly onChange: NonNullable<IServerListConfigControllerOptions['onChange']>;

  public readonly persistent: boolean;

  constructor(options: IServerListConfigControllerOptions) {
    this.onChange = options.onChange ?? noop;
    this.persistent = options.persitent ?? true;

    const config = reviveServerListConfig(options.config);

    this.sendableConfig = this.persistent
      ? getSavedListConfig(config)
      : config;
    this.observableConfig = clone(this.sendableConfig);

    makeAutoObservable(this, {
      // @ts-expect-error private
      sendableConfig: false,
    });
  }

  public get(): IServerListConfig {
    return this.observableConfig;
  }

  public getSendable(): IServerListConfig {
    return this.sendableConfig;
  }

  public get filteringByAnyTag(): boolean {
    return Object.keys(this.observableConfig.tags).length > 0;
  }

  public readonly clearTagsFilter = () => {
    this.observableConfig.tags = {};
    this.sendableConfig.tags = {};

    this.triggerChange();
  };

  public get filteringByAnyLocale(): boolean {
    return Object.keys(this.observableConfig.locales).length > 0;
  }

  public readonly clearLocalesFilter = () => {
    this.observableConfig.locales = {};
    this.sendableConfig.locales = {};

    this.triggerChange();
  };

  public get searchText(): string {
    return this.observableConfig.searchText;
  }

  public get searchTextParsed(): ISearchTerm[] {
    return this.observableConfig.searchTextParsed;
  }

  public readonly setSearchText = (searchText: string) => {
    const searchTextParsed = parseSearchTerms2(searchText);

    this.observableConfig.searchText = searchText;
    this.observableConfig.searchTextParsed = searchTextParsed;

    this.sendableConfig.searchText = searchText;
    this.sendableConfig.searchTextParsed = searchTextParsed;

    this.triggerChange();
  };

  public get sortBy(): ServersListSortBy {
    return this.observableConfig.sortBy;
  }

  public readonly setSortByName = () => this.setSortBy(ServersListSortBy.Name);

  public readonly setSortByPlayers = () => this.setSortBy(ServersListSortBy.Players);

  public readonly setSortBy = (sortBy: ServersListSortBy) => {
    if (this.sortBy !== sortBy) {
      this.observableConfig.sortBy = sortBy;
      this.sendableConfig.sortBy = sortBy;

      this.triggerChange();
    } else {
      this.toggleSortDir();
    }
  };

  public readonly setSortByBoosts = () => {
    if (this.sortBy === ServersListSortBy.Boosts) {
      return;
    }

    this.observableConfig.sortBy = ServersListSortBy.Boosts;
    this.sendableConfig.sortBy = ServersListSortBy.Boosts;

    this.setSortDir(ServersListSortDir.Desc);
  };

  public get sortDir(): ServersListSortDir {
    return this.observableConfig.sortDir;
  }

  public readonly toggleSortDir = () => {
    this.setSortDir(this.sendableConfig.sortDir * -1);
  };

  public readonly setSortDir = (sortDir: ServersListSortDir) => {
    this.observableConfig.sortDir = sortDir;
    this.sendableConfig.sortDir = sortDir;

    this.triggerChange();
  };

  public get hideEmpty(): boolean {
    return this.observableConfig.hideEmpty;
  }

  public readonly setHideEmpty = (hide: boolean) => {
    this.observableConfig.hideEmpty = hide;
    this.sendableConfig.hideEmpty = hide;

    this.triggerChange();
  };

  public get hideFull(): boolean {
    return this.observableConfig.hideFull;
  }

  public readonly setHideFull = (hide: boolean) => {
    this.observableConfig.hideFull = hide;
    this.sendableConfig.hideFull = hide;

    this.triggerChange();
  };

  public getTag(tag: string): boolean | undefined {
    return this.observableConfig.tags[tag];
  }

  public toggleTag(tag: string) {
    switch (this.sendableConfig.tags[tag]) {
      case undefined: {
        this.observableConfig.tags[tag] = true;
        this.sendableConfig.tags[tag] = true;
        break;
      }
      case true: {
        this.observableConfig.tags[tag] = false;
        this.sendableConfig.tags[tag] = false;
        break;
      }
      case false: {
        delete this.observableConfig.tags[tag];
        delete this.sendableConfig.tags[tag];
        break;
      }
    }

    this.triggerChange();
  }

  public getLocale(locale: string): boolean | undefined {
    return this.observableConfig.locales[locale];
  }

  public toggleLocale(locale: string) {
    switch (this.sendableConfig.locales[locale]) {
      case undefined: {
        this.observableConfig.locales[locale] = true;
        this.sendableConfig.locales[locale] = true;
        break;
      }
      case true: {
        this.observableConfig.locales[locale] = false;
        this.sendableConfig.locales[locale] = false;
        break;
      }
      case false: {
        delete this.observableConfig.locales[locale];
        delete this.sendableConfig.locales[locale];
        break;
      }
    }

    this.triggerChange();
  }

  refresh() {
    this.onChange(this.sendableConfig);
  }

  reset() {
    this.observableConfig.hideEmpty = false;
    this.sendableConfig.hideEmpty = false;

    this.observableConfig.hideFull = false;
    this.sendableConfig.hideFull = false;

    this.observableConfig.locales = {};
    this.sendableConfig.locales = {};

    this.observableConfig.tags = {};
    this.sendableConfig.tags = {};

    this.observableConfig.searchText = '';
    this.sendableConfig.searchText = '';

    this.observableConfig.searchTextParsed = [];
    this.sendableConfig.searchTextParsed = [];

    this.triggerChange();
  }

  private triggerChange() {
    this.onChange(this.sendableConfig);

    requestIdleCallback(() => saveListConfig(this.sendableConfig));
  }
}

interface ISavedFiltersConfig {
  searchText?: string;
  hideEmpty?: boolean;
  hideFull?: boolean;
  capPing?: boolean;
  maxPing?: number;
}

interface ISavedTagsConfig {
  tagList?: Record<string, boolean>;
  localeList?: Record<string, boolean>;
}

export function reviveServerListConfig(config: IPartialServerListConfig): IServerListConfig {
  return {
    searchText: '',
    searchTextParsed: [],

    hideEmpty: false,
    hideFull: false,

    locales: {},
    tags: {},

    capPing: false,
    maxPing: 0,

    sortBy: DEFAULT_SORT_BY,
    sortDir: DEFAULT_SORT_DIR,

    ...config,
  };
}

function getSavedListConfig(config: IServerListConfig): IServerListConfig {
  try {
    const savedFiltersString = window.localStorage.getItem(LS_KEYS.FiltersPrefix + config.type);

    if (savedFiltersString) {
      const savedFilters = JSON.parse(savedFiltersString);

      if (isObject<ISavedFiltersConfig>(savedFilters)) {
        if (typeof savedFilters.searchText === 'string') {
          config.searchText = String(savedFilters.searchText);
          config.searchTextParsed = parseSearchTerms2(config.searchText);
        }

        if (typeof savedFilters.hideEmpty === 'boolean') {
          config.hideEmpty = Boolean(savedFilters.hideEmpty);
        }

        if (typeof savedFilters.hideFull === 'boolean') {
          config.hideFull = Boolean(savedFilters.hideFull);
        }

        if (typeof savedFilters.capPing === 'boolean') {
          config.capPing = Boolean(savedFilters.capPing);
        }

        if (typeof savedFilters.maxPing === 'number') {
          config.maxPing = Number(savedFilters.maxPing) || 0;
        }
      }
    }
  } catch (e) {
    // noop
  }

  try {
    const savedTagsString = window.localStorage.getItem(LS_KEYS.TagsPrefix + config.type);

    if (savedTagsString) {
      const savedTags = JSON.parse(savedTagsString);

      if (isObject<ISavedTagsConfig>(savedTags)) {
        const {
          tagList,
          localeList,
        } = savedTags;

        if (isObject<ISavedTagsConfig['tagList']>(tagList)) {
          config.tags = Object.fromEntries(
            Object.entries(tagList)
              .map(([tag, enabled]) => [String(tag), Boolean(enabled)])
              .filter(([tag]) => tag),
          );
        }

        if (isObject<ISavedTagsConfig['localeList']>(localeList)) {
          config.locales = Object.fromEntries(
            Object.entries(localeList)
              .map(([locale, enabled]) => [String(locale), Boolean(enabled)])
              .filter(([locale]) => locale),
          );
        }
      }
    }
  } catch (e) {
    // noop
  }

  return config;
}

function saveListConfig(config: IServerListConfig) {
  const searchText = config.searchTextParsed[0]?.type === 'address'
    ? ''
    : config.searchText;

  window.localStorage.setItem(
    LS_KEYS.FiltersPrefix + config.type,
    JSON.stringify({
      searchText,
      hideEmpty: config.hideEmpty,
      hideFull: config.hideFull,
      capPing: config.capPing,
      maxPing: config.maxPing,
    } as ISavedFiltersConfig),
  );

  window.localStorage.setItem(
    LS_KEYS.TagsPrefix + config.type,
    JSON.stringify({
      tagList: config.tags,
      localeList: config.locales,
    } as ISavedTagsConfig),
  );
}
