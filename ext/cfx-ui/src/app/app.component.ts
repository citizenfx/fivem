import { Component, OnInit } from '@angular/core';

import { Translation, LocaleService, TranslationService } from 'angular-l10n';

import { GameService } from './game.service';
import { TrackingService } from './tracking.service';

import { environment } from '../environments/environment';

@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	styleUrls: ['app.component.scss']
})
export class AppComponent extends Translation implements OnInit {
	overlayActive = false;

	constructor(public locale: LocaleService,
		public translation: TranslationService,
		private gameService: GameService,
		private trackingService: TrackingService) {
		super();

		this.locale.init();

		this.translation.init();

		this.gameService.init();

		this.gameService.languageChange.subscribe(value => {
			this.locale.setCurrentLanguage(value);
		})
	}

	ngOnInit() {
		this.classes = [
			environment.web ? 'webapp' : 'gameapp',
			(this.gameService.darkTheme) ? 'theme-dark' : 'theme-light'
		];

		this.gameService.darkThemeChange.subscribe(value => {
			this.classes[1] = (value) ? 'theme-dark' : 'theme-light'
		});

		const lang = this.gameService.language;
		if (lang && this.locale.getAvailableLanguages().includes(lang)) {
			this.locale.setCurrentLanguage(lang);
		}
	}



	classes: string[];
}
