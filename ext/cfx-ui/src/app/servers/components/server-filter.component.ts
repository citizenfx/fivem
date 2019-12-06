import { Component, OnInit, OnChanges, OnDestroy, Input, Output, EventEmitter, ChangeDetectionStrategy, ChangeDetectorRef } from '@angular/core';

import { Server } from '../server';

import { ServersService } from '../servers.service';

import { GameService } from '../../game.service';

import { Subject } from 'rxjs/Subject';

import 'rxjs/add/operator/throttleTime';

import { ServerFilters } from './server-filter-container';

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
export class ServerFilterComponent implements OnInit, OnChanges, OnDestroy {
    filters: ServerFilters = new ServerFilters();

    @Input()
    type: string;

    lastType = "";

    @Output()
    public filtersChanged = new EventEmitter<ServerFilters>();

    isRefreshing = false;
    wantsToBeRefreshing = false;

    lastSearchText = '';

    searchFocused = false;
    searchAutocomplete: ServerAutocompleteEntry[] = [];

    selectedCompletionIndex = -1;

    autocompleteIndex: {[type: string]: {[entry: string]: number}} = {};

    refreshEvent = new Subject<void>();

	public mouseState = false;
	public maxPingPercent = 0;
	private minPingLimit = 30;
    private maxPingLimit = 200;

    constructor(private serversService: ServersService, private gameService: GameService, private cdr: ChangeDetectorRef) {
        this.serversService
            .getReplayedServers()
            .filter(server => !server)
            .subscribe(server => this.isRefreshing = false);

        this.serversService
            .getReplayedServers()
            .filter(server => !!server)
            .subscribe(server => {
                this.addAutocompleteIndex(server);
            });

/*        this.refreshEvent
            .throttleTime(10000)
            .subscribe(() => { this.wantsToBeRefreshing = false; this.isRefreshing = true; this.serversService.refreshServers() });*/
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

                        const uniqueBits = bits.filter((v, i, a) => a.indexOf(v) === i && typeof(v) === 'string');
                        this.addAutoCompleteEntries(subKey, uniqueBits);
                    }

                    if (typeof fields === 'string') {
                        this.addAutoCompleteEntries(key, [ fields ]);
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
        this.filtersChanged.emit(this.filters);

        this.setDefaultAutocompleteFilters();

        document.onkeydown = (ev: KeyboardEvent) => {
            if (ev.keyCode >= 65 && ev.keyCode <= 90 && !ev.ctrlKey) {
                (<HTMLInputElement>document.querySelector('#searchBox')).focus();
            }
        };
    }

    ngOnDestroy() {

    }

    ngOnChanges() {
        if (this.type != this.lastType)
        {
            const storedFilters = localStorage.getItem(`sfilters:${this.type}`);

            if (storedFilters) {
                this.filters = {...<ServerFilters>JSON.parse(storedFilters)};
            } else {
                this.filters = new ServerFilters();
            }

            this.lastType = this.type;
        }

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

        this.filtersChanged.emit(this.filters);
        localStorage.setItem(`sfilters:${this.type}`, JSON.stringify(this.filters));
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
        if (entry && entry.completion !== '') {
            this.filters.searchText = this.filters.searchText.replace(/(\s?)([^\s]*)$/, (str, space) => space + entry.completion) + ' ';
            this.filtersChanged.emit(this.filters);

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
        this.ngOnChanges();
    }
	
	isPingCap() {
		return this.filters.capPing;
	}
	
	changeMaxPing($event, force) {
		if ( this.mouseState || force ) {
			let pingBar = document.getElementById("progress").clientWidth;
			let widthPercent = $event.offsetX/pingBar;
			let ping = (widthPercent*(this.maxPingLimit-this.minPingLimit))+this.minPingLimit;
			
			this.maxPingPercent = widthPercent*100;
			this.filters.maxPing = Math.floor(ping);
			this.filtersChanged.emit(this.filters);
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
}