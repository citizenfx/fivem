import { Component, OnInit } from '@angular/core';

import { Translation, LocaleService, TranslationService } from 'angular-l10n';

import { GameService } from './game.service';
import { TrackingService } from './tracking.service';

import { environment } from '../environments/environment';
import { Router } from '@angular/router';

@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	styleUrls: ['app.component.scss']
})
export class AppComponent extends Translation implements OnInit {
	overlayActive = false;
	minModeSetUp = false;
	changelogShown = false;

	classes: { [key: string]: boolean } = {};

	get minMode() {
		return this.gameService.inMinMode;
	}

	get bgImage(): string {
		return this.minMode ? 'url(' + this.gameService.minmodeBlob['art:backgroundImage'] + ')' : '';
	}

	constructor(public locale: LocaleService,
		public translation: TranslationService,
		public gameService: GameService,
		private trackingService: TrackingService,
		private router: Router) {
		super();

		this.locale.init();

		this.translation.init();

		this.gameService.init();

		this.gameService.languageChange.subscribe(value => {
			this.locale.setCurrentLanguage(value);
		})

		this.gameService.minModeChanged.subscribe((value: boolean) => {
			if (value) {
				delete this.classes['theme-light'];
				this.classes['minmode'] = true;
				this.classes['theme-dark'] = true;
				this.classes = { ...this.classes };
				this.router.navigate(['/minmode']);
			}

			this.minModeSetUp = true;
		});
	}

	ngOnInit() {
		this.classes = {};
		this.classes[environment.web ? 'webapp' : 'gameapp'] = true;
		this.classes[(this.gameService.gameName === 'rdr3') ?
			'theme-rdr3' :
			(this.gameService.darkTheme) ? 'theme-dark' : 'theme-light'] = true;
		this.classes['game-' + this.gameService.gameName] = true;
		this.classes['theRoot'] = true;

		this.gameService.darkThemeChange.subscribe(value => {
			if (this.gameService.gameName !== 'rdr3') {
				delete this.classes['theme-light'];
				delete this.classes['theme-dark'];

				this.classes[(value) ? 'theme-dark' : 'theme-light'] = true;
				this.classes = {
					...this.classes
				};
			}
		});

		const lang = this.gameService.language;
		if (lang && this.locale.getAvailableLanguages().includes(lang)) {
			this.locale.setCurrentLanguage(lang);
		}
	}

	openChangelog() {
		this.changelogShown = true;
	}

	closeChangelog() {
		this.changelogShown = false;
	}
}
