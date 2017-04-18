import { Component, OnInit } from '@angular/core';

import { Server, ServerIcon } from '../server';
import { ServersService } from '../servers.service';
import { ServerFilters } from './server-filter.component';

@Component({
    moduleId: module.id,
    selector: 'app-server-list',
    templateUrl: 'server-list.component.html',
    styleUrls: ['server-list.component.scss']
})
export class ServerListComponent implements OnInit {
    servers: Server[];
    localServers: Server[]; // temp value
    icons: ServerIcon[];

    filters: ServerFilters;

    constructor(private serverService: ServersService) {
        this.filters = new ServerFilters();
    }

    ngOnInit() {
        this.loadServers();
    }

    setFilters(filters: ServerFilters) {
        this.filters = {...filters};
    }

    loadServers() {
        this.serverService
            .getServers()
            .then(list => this.servers = list)
            .then(list => this.serverService.getIcons(list))
            .then(list => this.icons = list)
            .then(list => {
                for (const entry of list) {
                    const server = this.servers.find(a => a.address == entry.addr)
                    
                    if (server) {
                        server.iconUri = entry.icon;
                    }
                }
            });
    }
}
