import { IServerListConfig } from '../lists/types';
import { IPinnedServersConfig, IServerView } from '../types';

export interface IServerListSource {
  onServersFetchStart(cb: () => void): void;
  onServersFetchChunk(cb: (chunk: Array<IServerView>) => void): void;
  onServersFetchEnd(cb: (chunk: Array<IServerView>) => void): void;
  onServersFetchError(cb: (error: string) => void): void;

  init(): void;

  makeList(config: IServerListConfig): void;
  onList(type: string, cb: (list: string[]) => void): void;

  onIndex(cb: (index: IAutocompleteIndex) => void): void;

  setPinnedConfig(config: IPinnedServersConfig): void;
}

export interface IListableServerView {
  id: string;

  ping: number;

  players: number;
  isFull: boolean;
  isEmpty: boolean;

  searchableName: string;
  sortableName: string;

  tags: string[];
  tagsMap: Record<string, boolean>;
  locale: string;
  variables: Record<string, string>;

  premium: string;
  upvotePower: number;

  categories: Record<string, IAnyCategoryMatcher>;
}

interface ICategoryMatcher<Type extends string, Matchee> {
  type: Type;
  against: Matchee;
}

export type IStringCategoryMatcher = ICategoryMatcher<'string', string>;
export type IArrayCategoryMatcher = ICategoryMatcher<'array', string[]>;

export type IAnyCategoryMatcher = IStringCategoryMatcher | IArrayCategoryMatcher;

export interface IIndexItem {
  label: string;
  count: number;
}

export interface IAutocompleteIndexItem {
  count: number;
}
export interface IAutocompleteIndexTagItem extends IAutocompleteIndexItem {}
export interface IAutocompleteIndexLocaleItem extends IAutocompleteIndexItem {
  locale: string;
  country: string;
}

interface AutocompleteEntry<Item extends IAutocompleteIndexItem = IAutocompleteIndexItem> {
  sequence: string[];
  items: Record<string, Item>;
}

export interface IAutocompleteIndex {
  tag: AutocompleteEntry<IAutocompleteIndexTagItem>;
  locale: AutocompleteEntry<IAutocompleteIndexLocaleItem>;
}
