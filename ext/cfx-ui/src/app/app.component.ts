import { Component, OnInit, NgZone, Inject } from '@angular/core';

import { GameService } from './game.service';
import { TrackingService } from './tracking.service';

import { environment } from '../environments/environment';
import { Router, NavigationEnd } from '@angular/router';
import { interval } from 'rxjs';
import { startWith, map, distinctUntilChanged, filter, take, tap } from 'rxjs/operators';
import { ServersService } from './servers/servers.service';
import { L10N_LOCALE, L10nLocale, L10nTranslationService } from 'angular-l10n';
import { OverlayContainer } from '@angular/cdk/overlay';
import { getNavConfigFromUrl } from './nav/helpers';

@Component({
	selector: 'app-root',
	templateUrl: 'app.component.html',
	styleUrls: ['app.component.scss']
})
export class AppComponent implements OnInit {
	overlayActive = false;
	minModeSetUp = false;
	changelogShown = false;

	showSiteNavbar = !!environment.web;

	gameName = 'gta5';

	classes: { [key: string]: boolean } = {};

	get minMode() {
		return this.gameService.inMinMode;
	}

	get bgImage(): string {
		return this.minMode ? 'url(' + this.gameService.minmodeBlob['art:backgroundImage'] + ')' : '';
	}

	get splashScreen() {
		return this.router.url === '/';
	}

	constructor(@Inject(L10N_LOCALE) public locale: L10nLocale,
		public l10nService: L10nTranslationService,
		public gameService: GameService,
		private trackingService: TrackingService,
		private router: Router,
		private zone: NgZone,
		private serversService: ServersService,
		private overlayContainer: OverlayContainer,
	) {
		this.gameService.init();

		this.gameService.languageChange.subscribe(value => {
			this.l10nService.setLocale({ language: value });
		})

		this.gameService.minModeChanged.subscribe((value: boolean) => {
			if (value) {
				delete this.classes['theme-light'];
				this.classes['minmode'] = true;
				this.classes['theme-dark'] = true;
				this.classes = { ...this.classes };
				overlayContainer.getContainerElement().classList.add('theme-dark');
				this.router.navigate(['/minmode']);
			}

			this.minModeSetUp = true;
		});

		this.gameService.getConvar('ui_blurPerfMode').subscribe((value: string) => {
			delete this.classes['blur-noBackdrop'];
			delete this.classes['blur-noBlur'];

			if (value === 'none' || value === 'backdrop') {
				this.classes['blur-noBackdrop'] = true;
			}

			if (value === 'none') {
				this.classes['blur-noBlur'] = true;
			}

			// trigger change detection
			this.classes = {
				...this.classes
			};
		});

		this.gameName = gameService.gameName;

		router.events.subscribe(event => {
			const url = (<NavigationEnd>event).url;

			if (url) {
				const { withHomeButton } = getNavConfigFromUrl(url);

				this.classes['no-header-safe-zone'] = !withHomeButton;
				this.classes = { ...this.classes };
			}
		});

		let settled = false;
		const settle = () => {
			if (settled) {
				return;
			}

			settled = true;
			this.serversService.onInitialized();

			(<HTMLDivElement>document.querySelector('.booting')).style.opacity = '0';
			(<HTMLDivElement>document.querySelector('app-root')).style.opacity = '1';
		};

		if (environment.web || !environment.production) {
			setTimeout(() => {
				settle();
			}, 100);
			return;
		}

		// We will either show ui in 1.5s or earlier when it is ready
		// this way we can be sure we don't ever block ui with loader forever
		setTimeout(settle, 1500);

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
					tap(settle)
				).subscribe();
		});
	}

	ngOnInit() {
		const themeName = this.gameService.gameName === 'rdr3'
			? 'theme-rdr3'
			: (this.gameService.darkTheme ? 'theme-dark' : 'theme-light');

		this.classes = {};
		this.classes[environment.web ? 'webapp' : 'gameapp'] = true;
		this.classes[themeName] = true;

		this.overlayContainer.getContainerElement().classList.add(themeName);

		this.classes['game-' + this.gameService.gameName] = true;
		this.classes['theRoot'] = true;
		this.classes['no-header-safe-zone'] = !getNavConfigFromUrl(this.router.url).withHomeButton;

		this.gameService.darkThemeChange.subscribe(value => {
			if (this.gameService.gameName !== 'rdr3') {
				const overlayElement = this.overlayContainer.getContainerElement();

				overlayElement.classList.remove('theme-light');
				overlayElement.classList.remove('theme-dark');

				delete this.classes['theme-light'];
				delete this.classes['theme-dark'];

				const themeName = value
					? 'theme-dark'
					: 'theme-light';

				this.classes[themeName] = true;
				overlayElement.classList.add(themeName);

				this.classes = {
					...this.classes
				};
			}
		});

		const lang = this.gameService.language;
		if (lang && this.l10nService.getAvailableLanguages().includes(lang)) {
			this.l10nService.setLocale({ language: lang });
		}
	}
}
