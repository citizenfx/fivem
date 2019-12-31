import { Component, OnInit, OnChanges, Input, NgZone, Inject, PLATFORM_ID, ChangeDetectorRef,
    ChangeDetectionStrategy, ElementRef, ViewChild, AfterViewInit } from '@angular/core';
import { Server } from '../server';
import { PinConfigCached } from '../pins';
import { ServersListHeadingColumn } from './servers-list-header.component';
import { ServerFilterContainer } from './server-filter-container';
import { Subject } from 'rxjs/Subject';
import { environment } from '../../../environments/environment';
import { LocalStorage } from '../../local-storage';

import { isPlatformBrowser } from '@angular/common';

import { getCanonicalLocale } from './utils';

import 'rxjs/add/operator/throttleTime';
import { ServersService } from '../servers.service';

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

    private unSpin: number;

    @ViewChild('list') private list: ElementRef;

    private interactingUntil = 0;
    private inInteraction: boolean;
    isSpinning: boolean;

    sortOrder: string[];

    columns: ServersListHeadingColumn[];

    localServers: Server[];
    sortedServers: Server[];

    constructor(private zone: NgZone, @Inject(LocalStorage) private localStorage: any, @Inject(PLATFORM_ID) private platformId: any,
        public changeDetectorRef: ChangeDetectorRef, private serversService: ServersService) {
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

                    if (this.isSpinning) {
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
            }, 500);
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

    sortAndFilterServers(fromInteraction?: boolean) {
        const result = this.serversService.sortAndFilter({
            servers: this.servers.map(a => [ a.address, a.data ]),
            filters: this.filters,
            sortOrder: this.sortOrder,
            pinConfig: this.pinConfig.data,
            fromInteraction: fromInteraction
        }, (sortedServers: string[]) => {
            this.sortedServers = sortedServers.map(a => this.serversService.rawServers[a]).filter(a => !!a);
            this.changeDetectorRef.markForCheck();

            if (this.isSpinning && this.inInteraction) {
                this.inInteraction = false;

                if (this.unSpin) {
                    clearTimeout(this.unSpin);
                }

                this.unSpin = setTimeout(() => {
                    this.isSpinning = false;
                    this.changeDetectorRef.markForCheck();
                }, 250) as unknown as number;
            }

            this.localStorage.setItem('sortOrder', JSON.stringify(this.sortOrder));
        });

        if (!this.isSpinning && this.sortedServers && this.sortedServers.length > 0) {
            this.isSpinning = fromInteraction || false;
            this.inInteraction = this.isSpinning;
            this.changeDetectorRef.markForCheck();
        }
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

        this.sortAndFilterServers(true);
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
            this.sortAndFilterServers(true);
            this.lastFilters = this.filters;
        }
    }

    svTrack(index: number, serverRow: Server) {
        return serverRow.address;
    }
}
