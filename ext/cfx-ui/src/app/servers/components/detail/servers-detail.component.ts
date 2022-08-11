import { Component, OnInit, OnDestroy, ViewChildren,
	ChangeDetectorRef, Inject, PLATFORM_ID, ViewChild, ElementRef, AfterViewInit, QueryList } from '@angular/core';
import { Location } from '@angular/common';
import { ActivatedRoute, Router } from '@angular/router';
import { DomSanitizer } from '@angular/platform-browser';
import { Subject } from 'rxjs';

import { Server } from '../../server';

import { GameService } from '../../../game.service';
import { Tweet, TweetService } from '../../../home/tweet.service';

import { ServersService } from '../../servers.service';

import { isPlatformBrowser } from '@angular/common';
import { L10N_LOCALE, L10nLocale, L10nTranslationService } from 'angular-l10n';
import { ServerTagsService } from '../../server-tags.service';
import { environment } from 'environments/environment';
import { NgxMasonryComponent } from 'ngx-masonry';
import { startWith } from 'rxjs/operators';
import { FiltersService } from 'app/servers/filters.service';

class VariablePair {
	public key: string;
	public value: string;
}

@Component({
	moduleId: module.id,
	selector: 'app-servers-detail',
	templateUrl: 'servers-detail.component.html',
	styleUrls: ['servers-detail.component.scss']
})

export class ServersDetailComponent implements OnInit, AfterViewInit, OnDestroy {
	addr = '';
	lastAddr = '';
	server: Server;
	serverVariables: VariablePair[] = [];
	filterFuncs: { [key: string]: (pair: VariablePair) => VariablePair } = {};
	resourceCount = 0;
	resources: string[] = [];
	error = '';
	canRefresh = true;
	headerScrolled = false;
	padTop = '0px';
	negPadTop = '0px';
	apFeedInitialized = false;

	@ViewChild(NgxMasonryComponent) masonry: NgxMasonryComponent;
	@ViewChildren('content') content: QueryList<ElementRef<HTMLDivElement>>;
	@ViewChildren('banner') bannerList: QueryList<ElementRef<HTMLDivElement>>;
	@ViewChildren('header') headerList: QueryList<ElementRef<HTMLDivElement>>;

	header: ElementRef<HTMLDivElement> = null;
	banner: ElementRef<HTMLDivElement> = null;

	interval: ReturnType<typeof setInterval>;

	currentAddr = '';

	onFetchCB: () => void;

	addrEvent = new Subject<[string, number]>();

	disallowedVars = [
		'sv_enhancedHostSupport',
		'sv_licenseKeyToken',
		'sv_lan',
		'sv_maxClients',
		'gamename',
		'activitypubFeed',
		'premium',
		'locale',
		'tags',
		'onesync_enabled',
	];

	communityTweets: Tweet[] = [];

	@ViewChildren('resourcesPane')
	resourcesPane: QueryList<ElementRef<HTMLDivElement>>;

	@ViewChildren('playersPane')
	playersPane: QueryList<ElementRef<HTMLDivElement>>;

	collator = new Intl.Collator(undefined, {numeric: true, sensitivity: 'base'});

	get serverAddress() {
		if (this.server) {
			if (this.server.address) {
				const address = this.server.address;

				const m = address.match(/(.*)-(.*?)\.cfx\.re/i);

				if (m) {
					return `cfx.re/join/${m[2]}`;
				}

				const m2 = address.match(/^[a-z0-9]{6,}$/i);

				if (m2) {
					return `cfx.re/join/${m2[0]}`;
				}

				return address;
			}
		}

		return null;
	}

	get language() {
		const lang = this.server?.data?.vars?.locale;

		if (!lang) {
			return '';
		}

		return this.tagService.getLocaleDisplayName(lang);
	}

	get joinable() {
		return (!this.eol && !this.server.data?.private);
	}

	get privateLabel() {
        return this.translation.translate('#ServerDetail_PrivateDisable', null, this.locale.language);
    }

	get offlineLabel() {
        return this.translation.translate('#ServerDetail_OfflineDisable', null, this.locale.language);
    }

	get joinableLabel() {
		if (this.server.data?.private) {
			return this.privateLabel;
		}

		if (this.eol) {
			return this.eolLabel;
		}

		if (this.server.data?.fallback) {
			return this.offlineLabel;
		}

		return '';
	}

	get serverWarning() {
		if (this.server.data?.private) {
			return '#ServerDetail_PrivateWarning';
		}

		if (this.eol) {
			return '#ServerDetail_EOLWarning';
		}

		if (this.eos) {
			return '#ServerDetail_SupportWarning';
		}

		if (this.server.data?.fallback) {
			return '#ServerDetail_OfflineWarning';
		}

		return '';
	}

	get onesyncEnabled() {
		return this.server?.data?.vars?.onesync_enabled === 'true';
	}

	get tags() {
		return this.server?.data?.vars?.tags;
	}

	get playersSorted() {
		const players: any[] = (this.server?.data?.players || []).slice();
		const sorted = players.sort((a, b) => this.collator.compare(a.name, b.name));

		if (sorted.length > 75) {
			sorted.length = 75;
		}

		return sorted;
	}

    get eol() {
        const now = new Date().getTime();

        // Tue Jun 01 2021 00:00:00 GMT+0200
        // Servers can't be EOL until this date.
        if (now < 1622498400000) {
            return false;
        }

        const supportStatus = (this.server.data.support_status ?? 'supported');

        return (supportStatus === 'unknown' || supportStatus === 'end_of_life');
    }

    get eolLabel() {
        return this.translation.translate('#ServerDetail_EOLDisable', null, this.locale.language);
    }

    get eos() {
        const supportStatus = (this.server.data.support_status ?? 'supported');

        return (supportStatus === 'end_of_support' || (!this.eol && supportStatus === 'end_of_life'));
    }

	constructor(private gameService: GameService, private serversService: ServersService,
		private route: ActivatedRoute, private cdRef: ChangeDetectorRef, private sanitizer: DomSanitizer,
		private router: Router, @Inject(PLATFORM_ID) private platformId: any,
		private tagService: ServerTagsService, private tweetService: TweetService,
        private translation: L10nTranslationService, private location: Location,
		private filtersService: FiltersService,
		@Inject(L10N_LOCALE) public locale: L10nLocale) {
		this.filterFuncs['sv_scriptHookAllowed'] = (pair) => {
			return {
				key: '#ServerDetail_ScriptHook',
				value: (pair.value === 'true') ? '#Yes' : '#No'
			};
		};
		this.filterFuncs['sv_enforceGameBuild'] = (pair) => {
			let value = pair.value;

			if (this.gameService.gameName === 'gta5') {
				if (pair.value === '2060') {
					value = 'Los Santos Summer Special';
				} else if (pair.value === '2189' || pair.value === '2215' || pair.value === '2245') {
					value = 'Cayo Perico Heist';
				} else if (pair.value === '2372') {
					value = 'Los Santos Tuners';
				} else if (pair.value === '2545' || pair.value === '2612') {
					value = 'The Contract';
				} else if (pair.value === '2699') {
					value = 'The Criminal Enterprises';
				}
			}

			return {
				key: '#ServerDetail_DLCLevel',
				value
			};
		};

		this.route.params.subscribe(params => {
			this.currentAddr = params['addr'];

			this.updateServer();
		});
	}

	get premiumName() {
		return this.serversService.getNameForPremium(this.server.premium);
	}

	isPinned(server: Server) {
		return this.filtersService.pinConfig.pinnedServers.has(server?.address);
	}

	get canHaveReviews() {
		return this.server.data?.vars?.can_review || this.isPinned(this.server);
	}

	private updateServer() {
		this.serversService.getServer(this.currentAddr)
			.then(a => {
				this.server = a;

				if (!a) {
					return;
				}

				this.fetchApFeed();

				const resources = (<string[]>(a?.data?.resources ?? []))
					.filter(res => res !== '_cfx_internal' && res !== 'hardcap' && res !== 'sessionmanager');

				this.resources = resources.sort(this.collator.compare);

				this.resourceCount = resources.length;

				this.serverVariables = Object.entries((a?.data.vars ?? {}) as { [key: string]: string })
					.map(([key, value]) => ({ key, value }))
					.filter(({ key }) => this.disallowedVars.indexOf(key) < 0)
					.filter(({ key }) => key.indexOf('banner_') < 0)
					.filter(({ key }) => key.indexOf('sv_project') < 0)
					.filter(({ key }) => key.indexOf('can_review') < 0)
					.filter(({ key }) => key.toLowerCase().indexOf('version') < 0)
					.filter(({ key }) => key.toLowerCase().indexOf('uuid') < 0)
					.filter(({ key, value }) => key !== 'sv_enforceGameBuild' || (value !== '1604' && value !== '1311'))
					.filter(({ key, value }) => key !== 'sv_scriptHookAllowed' || value === 'true')
					.map(pair => this.filterFuncs[pair.key] ? this.filterFuncs[pair.key](pair) : pair);
			});
	}

	openOwner() {
		this.gameService.openUrl(this.server?.data?.ownerProfile ?? '');
	}

	trackPlayer(index: number, player: any) {
		return player.name + ' ' + player.identifiers[0];
	}

	refreshServer() {
		if (this.canRefresh && isPlatformBrowser(this.platformId)) {
			this.updateServer();

			this.canRefresh = false;
			setTimeout(() => {
				this.canRefresh = true;
			}, 2000);
		}
	}

	goBack() {
		if (environment.web) {
			this.router.navigate(['/', 'servers']);
		} else {
			this.location.back();
		}
	}

	attemptConnect() {
        if (this.eol) {
            return;
        }

		this.gameService.connectTo(this.server);
	}

	isFavorite() {
		if (this.server) {
			return this.gameService.isMatchingServer('favorites', {
				EndPoint: this.server.address,
				Data: this.server.data
			});
		}

		return false;
	}

	addFavorite() {
		if (this.server) {
			this.gameService.toggleListEntry('favorites', this.server, true);
		}
	}

	removeFavorite() {
		if (this.server) {
			this.gameService.toggleListEntry('favorites', this.server, false);
		}
	}

	ngOnInit() {
		if (isPlatformBrowser(this.platformId)) {
			this.interval = setInterval(() => this.updateServer(), 45000);
		}

		this.communityTweets = [];

		this.fetchApFeed();
	}

	private fetchApFeed() {
		if (!this.server || this.apFeedInitialized) {
			return;
		}

		this.apFeedInitialized = true;

		const apFeed = this.server?.data?.vars?.activitypubFeed;

		if (apFeed) {
			this.tweetService
				.getActivityTweets([apFeed])
				.subscribe(tweet => {

					this.communityTweets = [
						tweet,
						...this.communityTweets
					];

					this.communityTweets.sort((a: Tweet, b: Tweet) => b.date.valueOf() - a.date.valueOf());

					setTimeout(() => {
						this.masonry?.reloadItems();
						this.masonry?.layout();
					}, 100);
				});
		}
	}

	ngAfterViewInit() {
		this.resourcesPane.changes.pipe(startWith(0)).subscribe((_) => {
			if (this.resourcesPane.first) {
				this.setupScroll(this.resourcesPane.first.nativeElement);
			}
		});

		this.playersPane.changes.pipe(startWith(0)).subscribe((_) => {
			if (this.playersPane.first) {
				this.setupScroll(this.playersPane.first.nativeElement);
			}
		});

		this.bannerList.changes.pipe(startWith(0)).subscribe((_) => {
			this.banner = this.bannerList.first;
		});

		this.headerList.changes.pipe(startWith(0)).subscribe(_ => {
			this.header = this.headerList.first;
		});

		this.content.changes.pipe(startWith(0)).subscribe(_ => {
			const element = this.content.first?.nativeElement;

			element?.addEventListener('scroll', () => {
				const bannerHeight = this.banner?.nativeElement?.clientHeight;
				const refHeight = this.header?.nativeElement?.clientHeight;
				this.headerScrolled = element.scrollTop >= bannerHeight;

				if (this.headerScrolled) {
					if (this.padTop === '0px') {
						this.padTop = (refHeight + 1) + 'px';
						this.negPadTop = (refHeight - bannerHeight) + 'px';
					}
				} else {
					this.padTop = '0px';
					this.negPadTop = '0px';
				}
			});
		});
	}

	setupScroll(element: HTMLDivElement) {
		if (!element) {
			return;
		}

		element.addEventListener('wheel', (wheelEvent) => {
			if (wheelEvent.deltaY !== 0) {
				element.scrollLeft += wheelEvent.deltaY;
				wheelEvent.preventDefault();
			}
		});
	}

	ngOnDestroy() {
		if (this.interval) {
			clearInterval(this.interval);
		}
	}

	trackResource(i: number, item: string) {
		return item;
	}

	openLink(event: MouseEvent) {
		event.preventDefault();

		for (const pathElement of event.composedPath()) {
			const element = (pathElement as HTMLElement);

			if (element.localName === 'a') {
				this.gameService.openUrl(element.getAttribute('href'));
			}
		}

		return false;
	}
}
