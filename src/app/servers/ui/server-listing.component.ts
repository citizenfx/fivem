import { Component, OnInit, OnChanges, Input } from '@angular/core';
import { Server } from '../server';

import { ServerHeadingColumn } from './server-heading.component';

@Component({
    moduleId: module.id,
    selector: 'app-server-listing',
    templateUrl: 'server-listing.component.html',
    styleUrls: ['server-listing.component.scss']
})

export class ServerListingComponent implements OnInit, OnChanges {
    @Input()
    private servers: Server[];

    private sortOrder: string[];

    get columns(): ServerHeadingColumn[] {
        return [
            {
                column: 'icon',
                label: ''
            },
            {
                column: 'name',
                label: '#ServerList_Name'
            },
            {
                column: 'players',
                label: '#ServerList_Players'
            },
            {
                column: 'ping',
                label: '#ServerList_Ping'
            }
        ];
    }

    private sortedServers: Server[];

    constructor() {
        this.servers = [];
        this.sortOrder = ['ping', '-'];
    }

    sortServers() {
        const servers = (this.servers || []).concat();

        servers.sort((a, b) => {
            const sortChain = (...stack: ((a: Server, b: Server) => number)[]) => {
                for (const entry of stack) {
                    const retval = entry(a, b);

                    if (retval != 0) {
                        return retval;
                    }
                }

                return 0;
            };

            const sortSortable = (sortable: string[]) => {
                const name = sortable[0];
                const invert = (sortable[1] == '-');

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

            return sortChain(
                sortSortable(this.sortOrder),
                sortSortable(['ping', '-'])
            );
        });

        this.sortedServers = servers;
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

        this.sortServers();
    }

    ngOnInit() { }

    ngOnChanges() {
        this.sortServers();
    }
}
