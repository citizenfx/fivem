import { Injectable, Inject } from "@angular/core";
import { Subject, BehaviorSubject } from "rxjs";
import { ServerSorting, ServerFilters, ServerTags, ServerSortBy, ServerSortDirection } from "./components/filters/server-filter-container";
import { PinConfig, PinConfigCached } from "./pins";
import { ServersService } from "./servers.service";
import { LocalStorage } from "app/local-storage";
import { Server } from "./server";
import { GameService } from "app/game.service";

export class ServerAutocompleteEntry {
	public name = '';
	public description = '';
	public example = '';
	public completion = '';
};

export interface SearchAutocompleteIndex {
	[type: string]: {
		[entry: string]: number,
	},
};

@Injectable()
export class FiltersService {
	filtersUpdates: Subject<ServerFilters> = new Subject();
	tagsUpdates: Subject<ServerTags> = new Subject();
	pinConfigUpdates: Subject<PinConfig> = new Subject();
	sortOrderUpdates: Subject<ServerSorting> = new Subject();

	filters: ServerFilters;
	tags: ServerTags;
	pinConfig: PinConfigCached;
	sortOrder: ServerSorting;

	sortOrderPerType: { [key: string]: ServerSorting } = {};

	sortingInProgress: BehaviorSubject<boolean> = new BehaviorSubject(false);

	sortedServersUpdate: Subject<Server[]> = new Subject();
	sortedServers: Server[] = [];

	autocompleteIndex: SearchAutocompleteIndex = {};
	autocompleteIndexUpdate: Subject<SearchAutocompleteIndex> = new BehaviorSubject(this.autocompleteIndex);

	private type: string;
	private _sortingsInProgress = 0;
	private filtersDebouncer;
	private sortingPending = false;

	constructor(
		private serversService: ServersService,
		@Inject(LocalStorage) private localStorage: any,
		private gameService: GameService,
	) {
		this.serversService.loadPinConfig()
			.then(pinConfig => {
				this.pinConfig = new PinConfigCached(pinConfig);
			});

		this.serversService
			.getReplayedServers()
			.subscribe(server => {
				if (server) {
					this.addAutocompleteIndex(server);
				}
			});

		this.serversService.serversLoadedUpdate.subscribe((loaded) => {
			if (loaded && this.sortingPending) {
				this.sortingPending = false;
				this.sortAndFilterServers(false);
			}
		});
	}

	addAutocompleteIndex(server: Server) {
		for (const list of [server.data, server.data.vars]) {
			for (const entryName in list) {
				if (list.hasOwnProperty(entryName)) {
					const key = entryName;
					const fields = list[entryName];

					if (key.endsWith('s')) {
						const bits: string[] = [];

						const subKey = key.substr(0, key.length - 1);

						if (Array.isArray(fields)) {
							for (const item of fields) {
								bits.push(item);
							}
						} else if (typeof fields === 'object') {
							const values = Object.keys(fields);

							for (const item of values) {
								bits.push(item);
							}
						} else if (typeof fields === 'string') {
							for (const item of fields.split(',')) {
								bits.push(item.trim());
							}
						}

						const uniqueBits = bits.filter((v, i, a) => a.indexOf(v) === i && typeof (v) === 'string');
						this.addAutoCompleteEntries(subKey, uniqueBits);
					}

					if (typeof fields === 'string') {
						this.addAutoCompleteEntries(key, [fields]);
					}
				}
			}
		}
	}

	addAutoCompleteEntries(key: string, list: string[]) {
		if (!this.autocompleteIndex[key]) {
			this.autocompleteIndex[key] = {};
		}

		for (const entry of list) {
			const lowerEntry = entry.toLowerCase();

			if (!this.autocompleteIndex[key][lowerEntry]) {
				this.autocompleteIndex[key][lowerEntry] = 1;
			} else {
				++this.autocompleteIndex[key][lowerEntry];
			}
		}

		this.autocompleteIndexUpdate.next(this.autocompleteIndex);
	}

	setType(type: string) {
		const lastType = this.type;
		this.type = type;

		if (lastType !== type) {
			const storedFilters = this.localStorage.getItem(`sfilters:${type}`);
			if (storedFilters) {
				this.filters = { ...<ServerFilters>JSON.parse(storedFilters) };
			} else {
				this.filters = new ServerFilters();
			}

			const storedTags = this.localStorage.getItem(`stags:${type}`);
			if (storedTags) {
				this.tags = { ...<ServerTags>JSON.parse(storedTags) };
			} else {
				this.tags = new ServerTags();
			}

			if (this.sortOrderPerType[type]) {
				this.sortOrder = this.sortOrderPerType[type];
			} else {
				this.sortOrder = [ServerSortBy.Boosts, ServerSortDirection.Desc];
			}

			this.tagsUpdates.next(this.tags);
			this.filtersUpdates.next(this.filters);
			this.sortOrderUpdates.next(this.sortOrder);

			this.sortAndFilterServers(false);
		}
	}

	setSortOrder(ord: ServerSorting) {
		this.sortOrderUpdates.next(ord);
		this.sortOrder = ord;

		this.sortOrderPerType[this.type] = ord;

		this.sortAndFilterServers(true);
	}

	setFilters(filters: ServerFilters) {
		this.filtersUpdates.next(filters);
		this.filters = filters;

		this.localStorage.setItem(`sfilters:${this.type}`, JSON.stringify(this.filters));

		this.sortAndFilterServers(true);
	}

	setFiltersDebounced(filters: ServerFilters) {
		this.filtersUpdates.next(filters);
		this.filters = filters;

		this.localStorage.setItem(`sfilters:${this.type}`, JSON.stringify(this.filters));

		if (this.filtersDebouncer) {
			clearTimeout(this.filtersDebouncer);
		}

		this.filtersDebouncer = setTimeout(() => {
			this.filtersDebouncer = null;
			this.sortAndFilterServers(true);
		}, 150);
	}

	setTags(tags: ServerTags) {
		this.tagsUpdates.next(tags);
		this.tags = tags;

		this.localStorage.setItem(`stags:${this.type}`, JSON.stringify(this.tags));

		this.sortAndFilterServers(true);
	}

	sortAndFilterServers(fromInteraction?: boolean) {
		if (Object.values(this.serversService.servers).length === 0) {
			this.sortingPending = true;
			return;
		}

		const willSort = this.serversService.sortAndFilter({
			filters: {
				filters: this.filters,
				tags: this.tags,
			},
			sortOrder: this.sortOrder,
			pinConfig: this.pinConfig.data,
			fromInteraction: fromInteraction
		}, (sortedServers: string[]) => {
			this.sortedServers = sortedServers
				.map(a => this.serversService.servers[a])
				.filter((server) => {
					if (!server) {
						return false;
					}

					const matching = this.gameService.isMatchingServer(this.type, server);

					return matching;
				});
			this.sortedServersUpdate.next(this.sortedServers);

			this.finishSorting();
		});

		if (willSort) {
			this.beginSorting();
		}
	}

	private beginSorting() {
		if (this._sortingsInProgress === 0) {
			this.sortingInProgress.next(true);
		}

		this._sortingsInProgress++;
	}

	private finishSorting() {
		this._sortingsInProgress--;

		if (this._sortingsInProgress === 0) {
			this.sortingInProgress.next(false);
		}
	}
}
