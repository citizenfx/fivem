import { Component, Input, ViewChild } from '@angular/core';

import { ContextMenuComponent } from 'ngx-contextmenu';

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

    removeFavorite() {
        this.gameService.toggleListEntry('favorites', this.server, false);
    }
}