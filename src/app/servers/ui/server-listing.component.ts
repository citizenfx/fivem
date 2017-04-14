import { Component, OnInit, Input } from '@angular/core';
import { Server } from '../server';

@Component({
    moduleId: module.id,
    selector: 'app-server-listing',
    templateUrl: 'server-listing.component.html',
    styleUrls: ['server-listing.component.scss']
})

export class ServerListingComponent implements OnInit {
    @Input()
    private servers: Server[];

    //private sortedServers: Server[];

    get sortedServers() {
        return this.servers;
    }

    constructor() { }

    ngOnInit() { }
}
