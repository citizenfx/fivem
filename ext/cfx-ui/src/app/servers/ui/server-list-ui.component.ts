import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { Server, ServerIcon, PinConfig } from '../server';
import { ServersService } from '../servers.service';
import { ServerFilters } from './server-filter.component';

import { GameService } from '../../game.service';

@Component({
    moduleId: module.id,
    selector: 'app-server-list-ui',
    templateUrl: 'server-list-ui.component.html',
    styleUrls: ['server-list-ui.component.scss']
})
export class ServerListUiComponent implements OnInit {
    servers: Server[];
    localServers: Server[]; // temp value
    icons: ServerIcon[];

    pinConfig: PinConfig;

    filters: ServerFilters;

    type: string;

    constructor(private serverService: ServersService, private gameService: GameService, private route: ActivatedRoute) {
        this.filters = new ServerFilters();
        this.pinConfig = new PinConfig();
    }

    ngOnInit() {
        this.type = this.route.snapshot.data.type;

        this.loadServers();
    }

    setFilters(filters: ServerFilters) {
        this.filters = {...filters};
    }

    filterType(list: Server[]) {
        return list.filter(a => this.gameService.isMatchingServer(this.type, a));
    }

    loadServers() {
        this.serverService
            .getServers()
            .then(list => this.filterType(list))
            .then(list => this.servers = list)
            .then(list => this.gameService.pingServers(list))
            .then(list => this.serverService.getIcons(list))
            .then(list => this.icons = list)
            .then(list => {
                for (const entry of list) {
                    const server = this.servers.find(a => a.address == entry.addr)
                    
                    if (server) {
                        server.iconUri = entry.icon;
                    }
                }
            })
            .then(() => this.serverService.loadPinConfig())
            .then(pinConfig => this.pinConfig = pinConfig);
    }
}