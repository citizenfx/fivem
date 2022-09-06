export class ServerFilters {
	public searchText = '';
	public hideEmpty = false;
	public hideFull = false;
	public capPing = false;
	public maxPing = 0;
}

export class ServerTags {
	public tagList: { [key: string]: boolean } = {};
	public localeList: { [key: string]: boolean } = {};
}

export class ServerFilterContainer {
	public filters: ServerFilters;
	public tags: ServerTags;
}

export enum ServerSortBy {
	Boosts = 'upvotePower',
	Name = 'name',
	Players = 'players',
};

export enum ServerSortDirection {
	Asc = '+',
	Desc = '-',
};

export type ServerSorting = [ServerSortBy, ServerSortDirection];
