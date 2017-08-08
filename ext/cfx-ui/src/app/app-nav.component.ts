import { Component } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';
import { Router, NavigationEnd } from '@angular/router';
import { GameService } from './game.service';
import { Server } from './servers/server'
import { Translation, TranslationService } from 'angular-l10n';

@Component({
	moduleId:    module.id,
	selector:    'app-nav',
	templateUrl: 'app-nav.component.html',
	styleUrls:   ['app-nav.component.scss']
})
export class AppNavComponent extends Translation {
	nickname = '';
	devMode = false;
	hasSubNav = false;

	constructor(
		private gameService: GameService,
		public translation: TranslationService,
		private domSanitizer: DomSanitizer,
		router: Router
	) {
		super(translation);

		this.nickname = gameService.nickname;
		this.devMode = gameService.devMode;

		router.events.subscribe(event => {
			if ((<NavigationEnd>event).urlAfterRedirects) {
				this.hasSubNav = (<NavigationEnd>event).urlAfterRedirects.startsWith('/servers');
			}
		});

		gameService.nicknameChange.subscribe(value => this.nickname = value);
		gameService.devModeChange.subscribe(value => this.devMode = value);
	}

	connectToLocal() {
		(<any>window).invokeNative('connectTo', '127.0.0.1:30120');
	}

	exitGame() {
		this.gameService.exitGame();
	}
}
