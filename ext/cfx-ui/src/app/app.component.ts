import { Component, OnInit, NgZone } from '@angular/core';

import { Translation, LocaleService, TranslationService } from 'angular-l10n';

import { GameService } from './game.service';
import { TrackingService } from './tracking.service';

import { environment } from '../environments/environment';
import { Router, NavigationStart } from '@angular/router';
import { interval } from 'rxjs';
import { startWith, map, distinctUntilChanged, filter, take, tap, takeUntil } from 'rxjs/operators';
import { ServersService } from './servers/servers.service';

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
		private router: Router,
		private zone: NgZone,
		private serversService: ServersService) {
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

		// reused snippet from https://dev.to/herodevs/route-fully-rendered-detection-in-angular-2nh4
		this.zone.runOutsideAngular(() => {
			// Check very regularly to see if the pending macrotasks have all cleared
			interval(10)
				.pipe(
					startWith(0), // So that we don't initially wait
					// Turn the interval number into the current state of the zone
					map(() => !this.zone.hasPendingMacrotasks),
					// Don't emit until the zone state actually flips from `false` to `true`
					distinctUntilChanged(),
					// Filter out unstable event. Only emit once the state is stable again
					filter(stateStable => stateStable === true),
					// Complete the observable after it emits the first result
					take(1),
					tap(stateStable => {
						this.serversService.onInitialized();

						(<HTMLDivElement>document.querySelector('.spinny')).style.display = 'none';
						(<HTMLDivElement>document.querySelector('app-root')).style.opacity = '1';
					})
				).subscribe();
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
