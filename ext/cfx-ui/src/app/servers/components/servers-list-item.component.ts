import { Component, Input, ViewChild } from '@angular/core';
import { Router } from '@angular/router';

import { Server } from '../server';

import { GameService } from '../../game.service'

@Component({
    moduleId: module.id,
    selector: 'servers-list-item',
    templateUrl: 'servers-list-item.component.html',
    styleUrls: ['servers-list-item.component.scss']
})

export class ServersListItemComponent {
    @Input()
    server: Server;

    @Input()
    pinned = false;

    constructor(private gameService: GameService, private router: Router) { }

    attemptConnect(event: Event) {
        this.gameService.connectTo(this.server);

        event.stopPropagation();
    }

    showServerDetail() {
        this.router.navigate(['/', 'servers', 'detail', this.server.address]);
    }

    isFavorite() {
        return this.gameService.isMatchingServer('favorites', this.server);
    }

    toggleFavorite(event: Event) {
        if (this.isFavorite()) {
            this.removeFavorite();
        } else {
            this.addFavorite();
        }

        event.stopPropagation();
    }

    addFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, true);
    }

    removeFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, false);
    }
}