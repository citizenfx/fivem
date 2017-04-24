import { Component, Input, ViewChild } from '@angular/core';

import { ContextMenuComponent } from 'ngx-contextmenu';

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

    @ViewChild(ContextMenuComponent)
    menu: ContextMenuComponent;

    constructor(private gameService: GameService) {

    }

    attemptConnect() {
        this.gameService.connectTo(this.server);
    }

    isFavorite() {
        return this.gameService.isMatchingServer('favorites', this.server);
    }

    addFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, true);
    }

    delFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, false);
    }
}