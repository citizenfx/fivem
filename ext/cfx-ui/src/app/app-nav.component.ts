import { Component, Output, EventEmitter, Inject } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';
import { Router, NavigationEnd } from '@angular/router';
import { GameService } from './game.service';
import { Server } from './servers/server'
import { ChangelogService } from './changelogs.service';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';

@Component({
	moduleId:    module.id,
	selector:    'app-nav',
	templateUrl: 'app-nav.component.html',
	styleUrls:   ['app-nav.component.scss']
})
export class AppNavComponent {
	nickname = '';
	devMode = false;
	localhostPort = '';
	language = '';
	hasSubNav = false;
	isInSteam = false;
	brandingName = 'CitizenFX';
	gameName = 'gta5';
	changelogCount = 0;

	@Output()
	openChangelog = new EventEmitter();

	constructor(
		private gameService: GameService,
		@Inject(L10N_LOCALE) public locale: L10nLocale,
		private domSanitizer: DomSanitizer,
		private changelog: ChangelogService,
		router: Router
	) {
		this.nickname = gameService.nickname;
		this.devMode = gameService.devMode;
		this.localhostPort = gameService.localhostPort;
		this.language = gameService.language;

		this.brandingName = gameService.brandingName;
		this.gameName = gameService.gameName;

		this.changelog.getUnreadItems()
			.then(a => this.changelogCount = a);

		this.changelog.onRead.subscribe(() => this.changelogCount = 0);

		router.events.subscribe(event => {
			if ((<NavigationEnd>event).url) {
				this.hasSubNav = (<NavigationEnd>event).url.startsWith('/servers');
			}
		});

		gameService.signinChange.subscribe(value => {
			this.nickname = value.name;
			this.isInSteam = (value.type == "steam") ? true : false;
		});
		gameService.nicknameChange.subscribe(value => this.nickname = value);
		gameService.devModeChange.subscribe(value => this.devMode = value);
		gameService.localhostPortChange.subscribe(value => this.localhostPort = value);
		gameService.languageChange.subscribe(value => this.language = value);
	}

	connectToLocal() {
		(<any>window).invokeNative('connectTo', '127.0.0.1:' + (this.localhostPort || '30120') );
	}

	goThere() {
		(<any>window).invokeNative('enterGameplay', '');
		document.body.style.display = 'none';
	}

	exitGame() {
		this.gameService.exitGame();
	}
}
