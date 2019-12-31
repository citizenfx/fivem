import { Component, OnInit } from '@angular/core';
import { GameService } from 'app/game.service';
import { Server } from 'app/servers/server';
import { DomSanitizer } from '@angular/platform-browser';

@Component({
    moduleId: module.id,
    selector: 'app-minmode',
    templateUrl: 'minmode.component.html',
    styleUrls: ['./minmode.component.scss']
})

export class MinModeComponent implements OnInit {
    constructor(private gameService: GameService, private sanitizer: DomSanitizer) {

    }

    ngOnInit() {
        this.retry();
    }

    retry() {
        this.gameService.connectTo(
            Server.fromObject(
                this.sanitizer,
                this.gameService.minmodeBlob.serverURL,
                {
                    vars: {
                        banner_connecting: this.gameService.minmodeBlob['art:connectBanner']
                    }
                }
            )
        );
    }

    quit() {
        this.gameService.exitGame();
    }
}
