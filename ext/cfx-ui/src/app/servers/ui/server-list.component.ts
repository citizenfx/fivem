import { Component } from '@angular/core';

import { GameService } from '../../game.service';

@Component({
    moduleId: module.id,
    selector: 'app-server-list',
    templateUrl: 'server-list.component.html',
    styleUrls: ['server-list.component.scss']
})
export class ServerListComponent {
    constructor(private gameService: GameService) {

    }

    exitGame() {
        this.gameService.exitGame();
    }
}
