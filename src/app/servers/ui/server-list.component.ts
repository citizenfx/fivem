import { Component, OnInit } from '@angular/core';

import { Server } from '../server';
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
        this.serverService.getServers().then(list => this.servers = list);
    }
}
