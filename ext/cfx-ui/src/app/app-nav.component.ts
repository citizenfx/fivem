import { Component } from '@angular/core';
import { Router, NavigationEnd } from '@angular/router';
import { GameService } from './game.service';
import { Translation, TranslationService } from 'angular-l10n';

@Component({
	moduleId:    module.id,
	selector:    'app-nav',
	templateUrl: 'app-nav.component.html',
	styleUrls:   ['app-nav.component.scss']
})
export class AppNavComponent extends Translation {
	id = 'ololo';

	hasSubNav: boolean = false;

	constructor(
		private gameService: GameService,
		public translation: TranslationService,
		router: Router
	) {
		super(translation);

		router.events.subscribe(event => {
			if ((<NavigationEnd>event).urlAfterRedirects) {
				this.hasSubNav = (<NavigationEnd>event).urlAfterRedirects.startsWith('/servers');
			}
		});
	}

	exitGame() {
		this.gameService.exitGame();
	}
}
