import { Component, OnInit } from '@angular/core';

import { Server } from './server';
import { ServersService } from './servers.service';

@Component({
    moduleId: module.id,
    selector: 'app-server-list',
    templateUrl: 'server-list.component.html',
    styleUrls: ['server-list.component.scss']
})

export class ServerListComponent implements OnInit {
    servers: Server[];
    localServers: Server[]; // temp value

    constructor(private serverService: ServersService) { }

    ngOnInit() {
        this.loadServers();
    }

    loadServers() {
        this.serverService.getServers().then(list => this.servers = list);
    }
}
