import { Component, OnInit, OnDestroy, ViewChildren, ChangeDetectorRef, Inject, PLATFORM_ID } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { DomSanitizer } from '@angular/platform-browser';
import { Subject } from 'rxjs';

import { Server } from '../../server';

import { GameService } from '../../../game.service';
import { Tweet, TweetService } from '../../../home/tweet.service';

import { ServersService } from '../../servers.service';

import { isPlatformBrowser } from '@angular/common';
import { MetaService } from '@ngx-meta/core';
import { L10N_LOCALE, L10nLocale, L10nTranslationService } from 'angular-l10n';
import { ServerTagsService } from '../../server-tags.service';

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

export class ServersDetailComponent implements OnInit, OnDestroy {
	addr = '';
	lastAddr = '';
	server: Server;
	serverVariables: VariablePair[] = [];
	filterFuncs: { [key: string]: (pair: VariablePair) => VariablePair } = {};
	resourceCount = 0;
	resources: string[] = [];
	error = '';
	canRefresh = true;

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

	get onesyncEnabled() {
		return this.server?.data?.vars?.onesync_enabled === 'true';
	}

	get tags() {
		return this.server?.data?.vars?.tags;
	}

	get playersSorted() {
		const players = (this.server?.data?.players || []).slice();

		return players.sort((a, b) => this.collator.compare(a.name, b.name));
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
		private router: Router, @Inject(PLATFORM_ID) private platformId: any, private meta: MetaService,
		private tagService: ServerTagsService, private tweetService: TweetService,
        private translation: L10nTranslationService,
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
				} else if (pair.value === '2189' || pair.value === '2215') {
					value = 'Cayo Perico Heist';
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

	apFeedInitialized = false;

	private updateServer() {
		this.serversService.getServer(this.currentAddr)
			.then(a => {
				this.server = a;

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
					.filter(({ key }) => key.toLowerCase().indexOf('version') < 0)
					.filter(({ key }) => key.toLowerCase().indexOf('uuid') < 0)
					.filter(({ key, value }) => key !== 'sv_enforceGameBuild' || (value !== '1604' && value !== '1311'))
					.filter(({ key, value }) => key !== 'sv_scriptHookAllowed' || value === 'true')
					.map(pair => this.filterFuncs[pair.key] ? this.filterFuncs[pair.key](pair) : pair);

				this.meta.setTag('og:image', this.server?.iconUri);
				this.meta.setTag('og:type', 'website');
				this.meta.setTitle(this.server.hostname.replace(/\^[0-9]/g, ''));
				this.meta.setTag('og:description', `${this.server.currentPlayers} players on ${this.server.data.mapname}`);
				this.meta.setTag('og:site_name', 'FiveM');
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
		this.router.navigate(['/', 'servers']);
	}

	attemptConnect() {
        if (this.eol) {
            return;
        }

		this.gameService.connectTo(this.server);
	}

	isFavorite() {
		if (this.server) {
			return this.gameService.isMatchingServer('favorites', this.server);
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
				});
		}
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
