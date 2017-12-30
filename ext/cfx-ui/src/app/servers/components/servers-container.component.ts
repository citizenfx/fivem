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
            .filter(a => a && this.gameService.isMatchingServer(this.type, a));

        // add each new server to our server list
        typedServers.subscribe(server => {
            if (!this.servers[server.address]) {
                this.serversArray.push(server);
            } else {
                this.serversArray.splice(
                    this.serversArray.findIndex(a => a.address === server.address),
                    1,
                    server);
            }

            this.serversArray = this.serversArray.slice();

            this.servers[server.address] = server;
        });

        // ping new servers after a while
        typedServers
            .bufferTime(100, null, 250)
            .subscribe(servers => (servers.length > 0) ? this.gameService.pingServers(servers) : null);
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