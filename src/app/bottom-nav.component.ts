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
    overlayClosable = true;
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
            this.overlayMessageData = { serverName: a.address };
            this.showOverlay = true;
            this.overlayClosable = false;
        });

        this.gameService.connectFailed.subscribe(([server, message]) => {
            this.overlayTitle = '#Servers_ConnectFailed';
            this.overlayMessage = '#Servers_Message';
            this.overlayMessageData = { message };
            this.showOverlay = true;
            this.overlayClosable = true;
        });

        this.gameService.connectStatus.subscribe(a => {
            this.overlayTitle = '#Servers_Connecting';
            this.overlayMessage = '#Servers_Message';
            this.overlayMessageData = { message: a.message };
            this.showOverlay = true;
            this.overlayClosable = false;
        });
    }
}