import { Injectable, Inject } from "@angular/core";
import { Subject, BehaviorSubject } from "rxjs";
import { ServerSorting, ServerFilters, ServerTags, ServerSortBy, ServerSortDirection } from "./components/filters/server-filter-container";
import { PinConfig, PinConfigCached } from "./pins";
import { ServersService } from "./servers.service";
import { LocalStorage } from "app/local-storage";
import { Server } from "./server";
import { GameService } from "app/game.service";
import { master } from "./master";

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
	pinConfig = new PinConfigCached(null);
	sortOrder: ServerSorting;

	sortOrderPerType: { [key: string]: ServerSorting } = {};

	sortingInProgress: BehaviorSubject<boolean> = new BehaviorSubject(false);

	sortedServersUpdate: Subject<master.IServer[]> = new Subject();
	sortedServers: master.IServer[] = [];

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

		this.serversService.autoCompleteUpdate.subscribe((index) => {
			this.autocompleteIndex = index;
			this.autocompleteIndexUpdate.next(index);
		});

		this.serversService.serversLoadedUpdate.subscribe((loaded) => {
			if (loaded && this.sortingPending) {
				this.sortingPending = false;
				this.sortAndFilterServers(false);
			}
		});
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
			this.tags = new ServerTags();

			if (storedTags) {
				try {
					this.tags = { ...<ServerTags>JSON.parse(storedTags) };
				} catch {}
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
		if (this.serversService.servers.size === 0) {
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
			fromInteraction: fromInteraction,
			listType: this.type
		}, (sortedServers: string[]) => {
			this.sortedServers = sortedServers
				.map(a => this.serversService.deserializeServer(this.serversService.servers.get(a)))
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
