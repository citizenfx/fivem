import { Component, OnInit, Input, ChangeDetectionStrategy, ChangeDetectorRef, ViewChild, Inject } from '@angular/core';

import { Server } from '../../server';

import { ServersService } from '../../servers.service';

import { GameService } from '../../../game.service';

import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/throttleTime';

import { ServerFilters, ServerSorting, ServerSortBy, ServerSortDirection } from './server-filter-container';
import { DirectConnectBackendComponent } from '../../direct/direct-connect-backend.component';
import { LocalStorage } from '../../../local-storage';
import { FiltersService } from '../../filters.service';

class ServerAutocompleteEntry {
	public name = '';
	public description = '';
	public example = '';
	public completion = '';
}

@Component({
	moduleId: module.id,
	selector: 'app-server-filter',
	templateUrl: 'server-filter.component.html',
	styleUrls: ['server-filter.component.scss'],
	changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServerFilterComponent implements OnInit {
	filters: ServerFilters = new ServerFilters();

	gameName = 'gta5';

	@Input()
	type: string;

	@ViewChild('backend')
	private backend: DirectConnectBackendComponent;

	isRefreshing = false;
	wantsToBeRefreshing = false;

	lastSearchText = '';

	searchFocused = false;
	searchAutocomplete: ServerAutocompleteEntry[] = [];

	selectedCompletionIndex = -1;

	autocompleteIndex: { [type: string]: { [entry: string]: number } } = {};

	refreshEvent = new Subject<void>();

	devMode = false;
	localhostPort = '';

	sortOrder: ServerSorting;

	sortBy = ServerSortBy;
	sortDirection = ServerSortDirection;

	public mouseState = false;
	public maxPingPercent = 0;
	private minPingLimit = 30;
	private maxPingLimit = 200;

	get sortByBoosts() {
		return this.sortOrder[0] === ServerSortBy.Boosts;
	}

	get sortByName() {
		return this.sortOrder[0] === ServerSortBy.Name;
	}
	get sortByNameAsc() {
		return this.sortOrder[0] === ServerSortBy.Name && this.sortOrder[1] === ServerSortDirection.Asc;
	}
	get sortByNameDesc() {
		return this.sortOrder[0] !== ServerSortBy.Name || (
			this.sortOrder[0] === ServerSortBy.Name && this.sortOrder[1] === ServerSortDirection.Desc
		);
	}

	get sortByPlayers() {
		return this.sortOrder[0] === ServerSortBy.Players;
	}
	get sortByPlayersAsc() {
		return this.sortOrder[0] === ServerSortBy.Players && this.sortOrder[1] === ServerSortDirection.Asc;
	}
	get sortByPlayersDesc() {
		return this.sortOrder[0] !== ServerSortBy.Players || (
			this.sortOrder[0] === ServerSortBy.Players && this.sortOrder[1] === ServerSortDirection.Desc
		);
	}

	constructor(
		private serversService: ServersService,
		@Inject(LocalStorage) private localStorage: any,
		private gameService: GameService,
		private cdr: ChangeDetectorRef,
		private filtersService: FiltersService,
	) {
		this.gameName = gameService.gameName;
		this.devMode = gameService.devMode;
		this.localhostPort = gameService.localhostPort;

		this.sortOrder = filtersService.sortOrder;
		this.filters = filtersService.filters;

		gameService.devModeChange.subscribe(value => this.devMode = value);

		filtersService.sortOrderUpdates.subscribe((sortOrder) => {
			this.sortOrder = sortOrder;

			this.cdr.markForCheck();
		});

		filtersService.filtersUpdates.subscribe((filters) => {
			this.filters = filters;

			if (this.filters.searchText !== this.lastSearchText) {
				this.lastSearchText = this.filters.searchText;

				const autocompleteBitMatch = /\s?([^\s]*)$/.exec(this.lastSearchText)[1];

				if (autocompleteBitMatch.length === 0) {
					this.setDefaultAutocompleteFilters();
				} else {
					this.setAutocompleteFiltersFor(autocompleteBitMatch);
				}

				this.selectedCompletionIndex = -1;
			}

			this.cdr.markForCheck();
		});
	}

	updateSort(sortBy: ServerSortBy) {
		const [currentSortBy, currentSortDirection] = this.sortOrder;

		const newSortOrder: ServerSorting = [sortBy, currentSortDirection];

		switch (true) {
			case sortBy === ServerSortBy.Boosts: {
				if (currentSortBy !== ServerSortBy.Boosts) {
					newSortOrder[0] = ServerSortBy.Boosts;
					newSortOrder[1] = ServerSortDirection.Desc;
				}

				break;
			}
			case currentSortDirection === ServerSortDirection.Asc: {
				newSortOrder[0] = ServerSortBy.Boosts;
				newSortOrder[1] = ServerSortDirection.Desc;

				break;
			}
			case sortBy !== currentSortBy: {
				newSortOrder[0] = sortBy;
				newSortOrder[1] = ServerSortDirection.Desc;

				break;
			}
			case currentSortDirection === ServerSortDirection.Desc: {
				newSortOrder[1] = ServerSortDirection.Asc;

				break;
			}
			default: {}
		}

		if (newSortOrder[0] !== currentSortBy || newSortOrder[1] !== currentSortDirection) {
			this.sortOrder = newSortOrder;
			this.filtersService.setSortOrder(newSortOrder);
		}
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
	}

	ngOnInit() {
		this.setDefaultAutocompleteFilters();

		document.onkeydown = (ev: KeyboardEvent) => {
			if (ev.keyCode >= 65 && ev.keyCode <= 90 && !ev.ctrlKey) {
				(<HTMLInputElement>document.querySelector('#searchBox')).focus();
			}
		};

		setTimeout(() => {
			this.serversService
				.getReplayedServers()
				.subscribe(server => {
					return;

					if (server) {
						this.addAutocompleteIndex(server);
					} else {
						this.isRefreshing = false;
					}
				});
		}, 0);
	}

	setDefaultAutocompleteFilters() {
		this.searchAutocomplete = [
			{
				name: 'Variable',
				description: 'server info variables set on the server',
				example: 'var:scripthook, var:premium',
				completion: ''
			},
			{
				name: 'Tag',
				description: 'tags are easy to find things with',
				example: 'tag:default, tag:drifting, tag:zombie',
				completion: ''
			},
			{
				name: 'Negation',
				description: 'you can negate filters',
				example: '~roleplay, ~/dr.ft/, ~var:scripthook',
				completion: ''
			},
			{
				name: 'Regular expressions',
				description: 'for advanced users',
				example: '/r.le ?play/, /[0-9]{2}/',
				completion: ''
			},
			{
				name: '',
				description: '... or just enter part of a server name!',
				example: '',
				completion: ''
			}
		];
	}

	private quoteRe(text: string) {
		return text.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
	}

	setAutocompleteFiltersFor(match: string) {
		const parseList = (matchRe, dataArray) => {
			return dataArray
				.filter(entry => entry.data[0].indexOf(' ') === -1)
				.filter(entry => matchRe.test(entry.data[0]))
				.sort((a, b) => b.data[1] - a.data[1])
				.slice(0, 10)
				.map(entry => ({
					name: '',
					description: entry.key + ':' + entry.data[0],
					example: '',
					completion: entry.key + ':' + entry.data[0]
				}));
		};

		if (match.indexOf(':') !== -1) {
			const [key, value] = match.split(':');
			const index = this.autocompleteIndex[key];

			if (!index) {
				return;
			}

			const matchRe = new RegExp(this.quoteRe(value), 'i');

			const dataArray = Object.entries(index);

			this.searchAutocomplete = parseList(matchRe, dataArray.map(a => ({ key: key, data: a })));
		} else {
			const matchRe = new RegExp(this.quoteRe(match), 'i');

			let dataArray = [];

			if (this.autocompleteIndex['tag']) {
				dataArray = dataArray.concat(Object.entries(this.autocompleteIndex['tag']).map(a => ({ key: 'tag', data: a })));
			}

			if (this.autocompleteIndex['var']) {
				dataArray = dataArray.concat(Object.entries(this.autocompleteIndex['var']).map(a => ({ key: 'var', data: a })));
			}

			this.searchAutocomplete = parseList(matchRe, dataArray);
		}
	}

	acceptAutocomplete(entry: ServerAutocompleteEntry) {
		if (entry?.completion !== '') {
			this.filters.searchText = this.filters.searchText.replace(/(\s?)([^\s]*)$/, (str, space) => space + entry.completion) + ' ';
			this.filtersService.setFilters(this.filters);

			this.selectedCompletionIndex = -1;
			this.setDefaultAutocompleteFilters();
		}
	}

	onKeyPress(event: KeyboardEvent) {
		if (event.keyCode === 38) { // up
			this.selectedCompletionIndex--;

			if (this.selectedCompletionIndex < 0) {
				this.selectedCompletionIndex = this.searchAutocomplete.length - 1;
			}

			event.preventDefault();
		} else if (event.keyCode === 40) { // down
			this.selectedCompletionIndex++;

			if (this.selectedCompletionIndex >= this.searchAutocomplete.length) {
				this.selectedCompletionIndex = 0;
			}

			event.preventDefault();
		} else if (event.keyCode === 13 || event.keyCode === 9) { // enter
			this.acceptAutocomplete(this.searchAutocomplete[this.selectedCompletionIndex]);

			event.preventDefault();
		} else if (event.keyCode === 27) { // esc
			(<HTMLInputElement>event.srcElement).blur();

			event.preventDefault();
		}
	}

	onSearchFocus() {
		this.searchFocused = true;
	}

	onSearchBlur() {
		setTimeout(() => {
			this.searchFocused = false;

			this.cdr.markForCheck();
		}, 200);
	}

	updateFilters() {
		this.filtersService.setFilters(this.filters);
	}

	private debouncer;

	updateFiltersDebounced() {
		if (this.debouncer) {
			clearTimeout(this.debouncer);
		}

		this.debouncer = setTimeout(() => {
			this.debouncer = null;
			this.filtersService.setFilters(this.filters);
		}, 500);
	}

	isPingCap() {
		return this.filters.capPing;
	}

	changeMaxPing($event, force) {
		if (this.mouseState || force) {
			let pingBar = document.getElementById("progress").clientWidth;
			let widthPercent = $event.offsetX / pingBar;
			let ping = (widthPercent * (this.maxPingLimit - this.minPingLimit)) + this.minPingLimit;

			this.maxPingPercent = widthPercent * 100;
			this.filters.maxPing = Math.floor(ping);
			this.filtersService.setFilters(this.filters);
		}
	}

	refresh() {
		if (!this.wantsToBeRefreshing) {
			// comforting timeout for spam clicks, no debouncing though
			window.setTimeout(() => {
				this.wantsToBeRefreshing = false;
			}, 1000);

			this.wantsToBeRefreshing = true;
		}

		this.refreshEvent.next();
	}

	rentServer() {
		this.gameService.openUrl('https://zap-hosting.com/fivemigcl');
	}

	get beValid() {
		return this.backend?.isValid() ?? false;
	}

	connectToLocal() {
		(<any>window).invokeNative('connectTo', '127.0.0.1:' + (this.localhostPort || '30120'));
	}
}
