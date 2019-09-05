import { Component, OnInit, OnChanges, Input, NgZone, Inject, PLATFORM_ID, ChangeDetectorRef,
    ChangeDetectionStrategy, ElementRef, ViewChild, AfterViewInit } from '@angular/core';
import { Server, PinConfigCached } from '../server';
import { ServersListHeadingColumn } from './servers-list-header.component';
import { ServerFilterContainer } from './server-filter.component';
import { Subject } from 'rxjs/Subject';
import { environment } from '../../../environments/environment';
import { LocalStorage } from '../../local-storage';

import { isPlatformBrowser } from '@angular/common';

import { getCanonicalLocale } from './utils';

import 'rxjs/add/operator/throttleTime';

@Component({
    moduleId: module.id,
    selector: 'servers-list',
    templateUrl: 'servers-list.component.html',
    styleUrls: ['servers-list.component.scss'],
    changeDetection: ChangeDetectionStrategy.OnPush
})
export class ServersListComponent implements OnInit, OnChanges, AfterViewInit {
    @Input()
    private servers: Server[];

    @Input()
    private filters: ServerFilterContainer;

    @Input()
    private pinConfig: PinConfigCached;

    private lastFilters: ServerFilterContainer;

    private subscriptions: { [addr: string]: any } = {};

    private lastLength: number;

    @ViewChild('list', { static: false }) private list: ElementRef;

    private interactingUntil = 0;

    sortOrder: string[];

    columns: ServersListHeadingColumn[];

    localServers: Server[];
    sortedServers: Server[];

    constructor(private zone: NgZone, @Inject(LocalStorage) private localStorage: any, @Inject(PLATFORM_ID) private platformId: any,
        public changeDetectorRef: ChangeDetectorRef) {
        this.servers = [];

        this.columns = [
            {
                column: 'icon',
                label: ''
            },
            {
                column: 'name',
                label: '#ServerList_Name',
                sortable: true
            },
            {
                column: 'actions',
                label: ''
            },
            {
                column: 'players',
                label: '#ServerList_Players',
                sortable: true
            },
            {
                column: 'ping',
                label: '#ServerList_Ping',
                sortable: true
            }
        ];

        const storedOrder = this.localStorage.getItem('sortOrder');

        //if (storedOrder) {
        //    this.sortOrder = JSON.parse(storedOrder);
        //} else {
        this.sortOrder = ['upvotePower', '-'];
        //}

        let changed = false;

        this.changeObservable.subscribe(() => {
            changed = true;
        });

        zone.runOutsideAngular(() => {
            setInterval(() => {
                if (changed) {
                    if (this.interactingUntil >= new Date().getTime()) {
                        return;
                    }

                    changed = false;

                    for (const server of (this.servers || [])) {
                        if (!this.subscriptions[server.address]) {
                            this.subscriptions[server.address] = server.onChanged.subscribe(a => this.changeSubject.next());
                        }
                    }

                    zone.run(() => {
                        this.sortAndFilterServers();
                    });
                }
            }, 250);
        });
    }

    isBrowser() {
        return isPlatformBrowser(this.platformId);
    }

    isPinned(server: Server) {
        if (!this.pinConfig || !this.pinConfig.pinnedServers) {
            return false;
        }

        return this.pinConfig.pinnedServers.has(server.address);
    }

    isPremium(server: Server) {
        return (server.data.vars && server.data.vars.premium !== undefined);
    }

    getPremium(server: Server) {
        if (!server.data.vars) {
            return '';
        }

        return server.data.vars.premium;
    }

    // to prevent auto-filtering while scrolling (to make scrolling feel smoother)
    updateInteraction() {
        this.interactingUntil = new Date().getTime() + 500;
    }

    private static quoteRe(text: string) {
        return text.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    }

    private buildSearchMatch(filterList: ServerFilterContainer) {
        const filters = filterList.filters;
        const searchText = filters.searchText;
        const filterFns: ((server: Server) => boolean)[] = [];

        const searchRe = /((?:~?\/.*?\/)|(?:[^\s]+))\s?/g;
        const categoryRe = /^([^:]*?):(.*)$/

        const isFalse = a => !a || a === 'false' || a === '0';

        let match: RegExpExecArray;

        while (match = searchRe.exec(searchText)) {
            let searchGroup = match[1];
            let invertSearch = false;

            if (searchGroup.startsWith('~')) {
                searchGroup = searchGroup.substring(1);
                invertSearch = true;
            }

            if (searchGroup.length < 2) {
                continue;
            }

            const categoryMatch = searchGroup.match(categoryRe);

            if (!categoryMatch) {
                const reString =
                    (searchGroup.match(/^\/(.+)\/$/)) ?
                        searchGroup.replace(/^\/(.+)\/$/, '$1')
                    :
                        ServersListComponent.quoteRe(searchGroup);

                try {
                    const re = new RegExp(reString, 'i');
                    filterFns.push(a => (invertSearch) ? !re.test(a.strippedname) : re.test(a.strippedname));
                } catch (e) {}
            } else {
                const category = categoryMatch[1];
                const match = new RegExp(ServersListComponent.quoteRe(categoryMatch[2]), 'i');

                filterFns.push(server => {
                    let result = false;

                    if (server.data[category]) {
                        result = match.test(String(server.data[category]));
                    } else if (server.data[category + 's']) {
                        const fields = server.data[category + 's'];

                        if (Array.isArray(fields)) {
                            result = fields.filter(a => match.test(String(a))).length > 0;
                        } else {
                            const values = Object.keys(fields).filter(a => match.test(String(a)));

                            if (values.length > 0) {
                                if (!isFalse(fields[values[0]])) {
                                    result = true;
                                }
                            }
                        }
                    } else if (server.data.vars[category + 's']) {
                        const fields = (<string>server.data.vars[category + 's']).split(',');

                        result = fields.filter(a => match.test(String(a))).length > 0;
                    } else if (server.data.vars[category]) {
                        result = match.test(String(server.data.vars[category]));
                    }

                    if (invertSearch) {
                        result = !result;
                    }

                    return result;
                });
            }
        }

        return (server: Server) => {
            for (const fn of filterFns) {
                if (!fn(server)) {
                    return false;
                }
            }

            return true;
        };
    }

    getFilter(filterList: ServerFilterContainer): (server: Server) => boolean {
        const nameMatchCallback = this.buildSearchMatch(filterList);
        const filters = filterList.filters;

        const hiddenByTags = (server: Server) => {
            const tagListEntries = (filterList.tags) ? Object.entries(filterList.tags.tagList) : [];

            if (tagListEntries.length > 0) {
                const tags =
                    (server && server.data && server.data.vars && server.data.vars.tags) ?
                        (<string>server.data.vars.tags)
                            .split(',')
                            .map(a => a.trim().toLowerCase())
                            .filter(a => a)
                        :
                            [];

                const tagSet = new Set<string>(tags);

                for (const [ tag, active ] of tagListEntries) {
                    if (active) {
                        if (!tagSet.has(tag)) {
                            return true;
                        }
                    } else {
                        if (tagSet.has(tag)) {
                            return true;
                        }
                    }
                }
            }

            return false;
        };

        const hiddenByLocales = (server: Server) => {
            const localeListEntries = (filterList.tags) ? Object.entries(filterList.tags.localeList) : [];

            let matchesLocales = true;

            if (localeListEntries.length > 0) {
                const sl = (server && server.data && server.data.vars && server.data.vars.locale
                    && getCanonicalLocale(server.data.vars.locale));

                matchesLocales = false;

                for (const [ locale, active ] of localeListEntries) {
                    if (active) {
                        if (sl === locale) {
                            matchesLocales = true;
                        }
                    } else {
                        if (sl === locale) {
                            return true;
                        }
                    }
                }
            }

            return !matchesLocales;
        };

        return (server) => {
            if (!nameMatchCallback(server)) {
                return false;
            }

            if (server.currentPlayers === 0 && filters.hideEmpty) {
                if (!this.isPinned(server) || !this.pinConfig.data.pinIfEmpty) {
                    return false;
                }
            }

            if (server.currentPlayers >= server.maxPlayers && filters.hideFull) {
                return false;
            }

            if (filters.capPing && (server.ping > filters.maxPing || typeof server.ping === 'string')) {
                return false;
            }

            if (filterList.tags.tagList) {
                if (hiddenByTags(server)) {
                    return false;
                }
            }

            if (filterList.tags.localeList) {
                if (hiddenByLocales(server)) {
                    return false;
                }
            }

            return true;
        }
    }

    sortAndFilterServers() {
        const servers = (this.servers || []).filter(this.getFilter(this.filters));

        const sortChain = (a: Server, b: Server, ...stack: ((a: Server, b: Server) => number)[]) => {
            for (const entry of stack) {
                const retval = entry(a, b);

                if (retval !== 0) {
                    return retval;
                }
            }

            return 0;
        };

        const sortSortable = (sortable: string[]) => {
            const name = sortable[0];
            const invert = (sortable[1] === '-');

            const sort = (a: Server, b: Server) => {
                const val1 = a.getSortable(name);
                const val2 = b.getSortable(name);

                if (val1 > val2) {
                    return 1;
                }

                if (val1 < val2) {
                    return -1;
                }

                return 0;
            };

            if (invert) {
                return (a: Server, b: Server) => -(sort(a, b));
            } else {
                return sort;
            }
        };

        const sortList = [
            (a: Server, b: Server) => {
                const aPinned = this.isPinned(a);
                const bPinned = this.isPinned(b);

                if (aPinned === bPinned) {
                    return 0;
                } else if (aPinned && !bPinned) {
                    return -1;
                } else if (!aPinned && bPinned) {
                    return 1;
                }
            },
            sortSortable(this.sortOrder),
            sortSortable(['upvotePower', '-']),
            sortSortable(['ping', '+']),
            sortSortable(['name', '+'])
        ];

        servers.sort((a, b) => {
            return sortChain(
                a,
                b,
                ...sortList
            );
        });

        this.sortedServers = servers;

        this.localStorage.setItem('sortOrder', JSON.stringify(this.sortOrder));
    }

    updateSorting(column: string) {
        if (this.sortOrder[0] != column) {
            this.sortOrder = [column, '+'];
        } else {
            this.sortOrder = [
                column,
                this.sortOrder[1] == '+' ? '-' : '+'
            ];
        }

        this.sortAndFilterServers();
    }

    ngOnInit() {
    }

    ngAfterViewInit() {
        const element = this.list.nativeElement as HTMLElement;

        this.zone.runOutsideAngular(() => {
            element.addEventListener('wheel', (e) => {
                this.updateInteraction();
            });
        });
    }

    changeSubject: Subject<void> = new Subject<void>();
    changeObservable = this.changeSubject.asObservable();

    ngOnChanges() {
        if (this.servers.length !== this.lastLength) {
            this.changeSubject.next();
            this.lastLength = this.servers.length;
        }

        if (this.filters !== this.lastFilters) {
            this.sortAndFilterServers();
            this.lastFilters = this.filters;
        }
    }

    svTrack(index: number, serverRow: Server) {
        return serverRow.address;
    }
}
