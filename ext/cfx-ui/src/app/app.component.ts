import {Component} from '@angular/core';

import {Translation, LocaleService, TranslationService} from 'angular-l10n';

import {GameService} from './game.service';

import localeEn from './locale-en.json';

@Component({
	selector:  'app-root',
	template:  `
				   <app-nav></app-nav>
				   <router-outlet></router-outlet>
				   <connecting-popup></connecting-popup>
			   `,
	styleUrls: ['app.component.scss']
})
export class AppComponent extends Translation {
	constructor(public locale: LocaleService,
				public translation: TranslationService,
				private gameService: GameService) {
		super(translation);

		this.locale.addConfiguration()
			.addLanguages(['en'])
			.defineLanguage('en');

		this.locale.init();

		this.translation.addConfiguration()
			.addTranslation('en', localeEn);

		this.translation.init();

		this.gameService.init();
	}
}
