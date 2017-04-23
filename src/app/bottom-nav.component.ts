import { Component, OnInit } from '@angular/core';
import { GameService } from './game.service';

import { Translation, TranslationService } from 'angular-l10n';

@Component({
    moduleId: module.id,
    selector: 'app-bottom-nav',
    templateUrl: 'bottom-nav.component.html',
    styleUrls: [ 'bottom-nav.component.scss' ]
})

export class BottomNavComponent extends Translation implements OnInit {
    showOverlay = false;
    overlayTitle: string;
    overlayMessage: string;
    overlayMessageData = {};

    constructor(private gameService: GameService, public translation: TranslationService) {
        super(translation);
    }

    ngOnInit() {
        this.gameService.connecting.subscribe(a => {
            this.overlayTitle = '#Servers_Connecting';
            this.overlayMessage = '#Servers_ConnectingTo';
            this.overlayMessageData = { server: a };
            this.showOverlay = true;
        });

        this.gameService.connectFailed.subscribe(([server, message]) => {
            this.overlayTitle = '#Servers_ConnectFailed';
            this.overlayMessage = message;
            this.showOverlay = true;
        });

        this.gameService.connectStatus.subscribe(a => {
            this.overlayTitle = '#Servers_Connecting';
            this.overlayMessage = a.message;
            this.overlayMessageData = {};
            this.showOverlay = true;
        });
    }
}