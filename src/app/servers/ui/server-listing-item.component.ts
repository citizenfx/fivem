import { Component, Input } from '@angular/core';

import { Server } from '../server';

import { GameService } from '../../game.service'

@Component({
    moduleId: module.id,
    selector: 'app-server-listing-item',
    templateUrl: 'server-listing-item.component.html',
    styleUrls: ['server-listing-item.component.scss']
})

export class ServerListingItemComponent {
    @Input()
    server: Server;

    constructor(private gameService: GameService) {

    }

    attemptConnect() {
        this.gameService.connectTo(this.server);
    }
}