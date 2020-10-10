import { Component, Inject } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';
import { Router, NavigationEnd } from '@angular/router';
import { GameService } from '../game.service';
import { ChangelogService } from '../changelogs.service';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';
import { getNavConfigFromUrl } from './helpers';
import { FiltersService } from 'app/servers/filters.service';

@Component({
	moduleId: module.id,
	selector: 'app-nav',
	templateUrl: 'app-nav.component.html',
	styleUrls: ['app-nav.component.scss']
})
export class AppNavComponent {
	nickname = '';
	language = '';
	hasSubNav = false;
	isInSteam = false;
	brandingName = 'CitizenFX';
	gameName = 'gta5';
	changelogCount = 0;

	hasBackground = false;
	hasHomeButton = false;

	homeButtonPulsating = false;

	exitConfirmationActive = false;

	constructor(
		private gameService: GameService,
		@Inject(L10N_LOCALE) public locale: L10nLocale,
		private domSanitizer: DomSanitizer,
		private changelog: ChangelogService,
		router: Router,
		private filtersService: FiltersService,
	) {
		this.nickname = gameService.nickname;
		this.language = gameService.language;

		this.brandingName = gameService.brandingName;
		this.gameName = gameService.gameName;

		this.changelog.getUnreadItems()
			.then(a => this.changelogCount = a);

		this.filtersService.sortingInProgress.subscribe((homeButtonPulsating) => {
			this.homeButtonPulsating = homeButtonPulsating;
		});

		this.changelog.onRead.subscribe(() => this.changelogCount = 0);

		const { withBackground, withHomeButton } = getNavConfigFromUrl(router.url);

		this.hasBackground = withBackground;
		this.hasHomeButton = withHomeButton;

		router.events.subscribe(event => {
			const url = (<NavigationEnd>event).url;

			if (url) {
				this.hasSubNav = url.startsWith('/servers');

				const { withBackground, withHomeButton } = getNavConfigFromUrl(url);

				this.hasBackground = withBackground;
				this.hasHomeButton = withHomeButton;
			}
		});

		gameService.signinChange.subscribe(value => {
			this.nickname = value.name;
			this.isInSteam = (value.type == "steam") ? true : false;
		});
		gameService.nicknameChange.subscribe(value => this.nickname = value);
		gameService.languageChange.subscribe(value => this.language = value);
	}

	goThere() {
	}

	exitGame() {
		this.gameService.exitGame();
	}
}
