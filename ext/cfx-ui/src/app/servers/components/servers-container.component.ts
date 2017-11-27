import { Component, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';

import { Server, ServerIcon, PinConfig } from '../server';
import { ServersService } from '../servers.service';
import { ServerFilters } from './server-filter.component';

import { GameService } from '../../game.service';

import { Observable } from 'rxjs/observable';

import 'rxjs/add/operator/bufferTime';

@Component({
    moduleId: module.id,
    selector: 'servers-container',
    templateUrl: 'servers-container.component.html',
    styleUrls: ['servers-container.component.scss']
})
export class ServersContainerComponent implements OnInit {
    servers: { [addr: string]: Server } = {};
    
    localServers: Server[]; // temp value
    icons: ServerIcon[];

    pinConfig: PinConfig;

    filters: ServerFilters;

    type: string;

    constructor(private serverService: ServersService, private gameService: GameService, private route: ActivatedRoute) {
        this.filters = new ServerFilters();
        this.pinConfig = new PinConfig();

        const typedServers = this.serverService
            .getServers()
            .filter(a => this.gameService.isMatchingServer(this.type, a));

        // add each new server to our server list
        typedServers.subscribe(server => this.servers[server.address] = server);

        typedServers.subscribe(() => this.serversArray = Object.entries(this.servers).map(([addr, server]) => server));

        // ping new servers after a while
        typedServers
            .subscribe(server => this.gameService.pingServers([server]));
    }

    serversArray: Server[] = [];

    ngOnInit() {
        this.type = this.route.snapshot.data.type;

        this.loadServers();
    }

    setFilters(filters: ServerFilters) {
        this.filters = {...filters};
    }

    loadServers() {
        this.serverService.loadPinConfig()
            .then(pinConfig => this.pinConfig = pinConfig);

        this.serverService.refreshServers();
    }
}