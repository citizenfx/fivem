import { Injectable, Inject } from "@angular/core";
import { Subject, BehaviorSubject } from "rxjs";
import { ServerSorting, ServerFilters, ServerTags, ServerSortBy, ServerSortDirection } from "./components/filters/server-filter-container";
import { PinConfig } from "./pins";
import { ServersService } from "./servers.service";
import { LocalStorage } from "app/local-storage";
import { Server } from "./server";

@Injectable()
export class FiltersService {
	filtersUpdates: Subject<ServerFilters> = new Subject();
	tagsUpdates: Subject<ServerTags> = new Subject();
	pinConfigUpdates: Subject<PinConfig> = new Subject();
	sortOrderUpdates: Subject<ServerSorting> = new Subject();

	filters: ServerFilters;
	tags: ServerTags;
	pinConfig: PinConfig;
	sortOrder: ServerSorting;

	sortOrderPerType: { [key: string]: ServerSorting } = {};

	sortingInProgress: BehaviorSubject<boolean> = new BehaviorSubject(false);

	sortedServersUpdate: Subject<Server[]> = new Subject();
	sortedServers: Server[] = [];
	servers: Server[] = [];

	private type: string;
	private _sortingsInProgress = 0;

	constructor(private serversService: ServersService, @Inject(LocalStorage) private localStorage: any) {
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

	setPinConfig(pinConfig: PinConfig) {
		this.pinConfig = pinConfig;
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

	setTags(tags: ServerTags) {
		this.tagsUpdates.next(tags);
		this.tags = tags;

		this.localStorage.setItem(`stags:${this.type}`, JSON.stringify(this.tags));

		this.sortAndFilterServers(true);
	}

	sortAndFilterServers(fromInteraction?: boolean) {
		if (this.servers.length === 0) {
			return;
		}

		const willSort = this.serversService.sortAndFilter({
			filters: {
				filters: this.filters,
				tags: this.tags,
			},
			sortOrder: this.sortOrder,
			pinConfig: this.pinConfig,
			fromInteraction: fromInteraction
		}, (sortedServers: string[]) => {
			this.sortedServers = sortedServers.map(a => this.serversService.rawServers[a]).filter(a => !!a);
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
