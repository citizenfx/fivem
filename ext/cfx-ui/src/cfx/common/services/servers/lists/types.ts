import { ISearchTerm } from 'cfx/base/searchTermsParser';

import { ServerListConfigController } from './ServerListConfigController';

export enum ServersListSortBy {
  Boosts = 'upvotePower',
  Name = 'name',
  Players = 'players',
}

export enum ServersListType {
  All = 'browse',
  Supporters = 'premium',
  Favorites = 'favorites',
  History = 'history',

  RegionalTop = 'regionalTop',
}

export enum ServerListSortDir {
  Asc = 1,
  Desc = -1,
}

export interface IServerListConfig {
  type: ServersListType;

  searchText: string;
  searchTextParsed: ISearchTerm[];

  hideEmpty: boolean;
  hideFull: boolean;
  capPing: boolean;
  maxPing: number;

  tags: Record<string, boolean>;
  locales: Record<string, boolean>;

  sortBy: ServersListSortBy;
  sortDir: ServerListSortDir;

  onlyPremium?: boolean;
  prioritizePinned?: boolean;
}

export type IPartialServerListConfig = Partial<IServerListConfig> & { type: ServersListType };

export interface IServersList {
  get sequence(): string[];

  refresh(): void;

  getConfig?(): ServerListConfigController | undefined;
}
