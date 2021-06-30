import { Component, OnInit, Inject, OnDestroy, ChangeDetectorRef, ɵSafeHtml } from '@angular/core';

import { Tweet, TweetService } from './tweet.service';

import { GameService } from '../game.service';
import { DiscourseService, DiscourseUser } from '../discourse.service';

import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { ServersService, HistoryServer, HistoryServerStatus } from 'app/servers/servers.service';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';
import { Server } from 'app/servers/server';

let cachedWelcomeMessage: SafeHtml;
let cachedServiceMessage: SafeHtml;

@Component({
	moduleId: module.id,
	selector: 'app-home',
	templateUrl: 'home.component.html',
	styleUrls: ['./home.component.scss']
})
export class HomeComponent implements OnInit, OnDestroy {
	officialTweets: Tweet[] = [];
	communityTweets: Tweet[] = [];

	currentAccount: DiscourseUser;

	serviceMessage: SafeHtml;
	welcomeMessage: SafeHtml;
	statusMessage = '';
	statusLevel = 0; // 0 = [unset], 1 = good, 2 = warn, 3 = bad
	statusInterval;

	brandingName: string;
	language = '';

	streamerMode = false;
	devMode = false;
	localhost = false;
	localhostName = '';
	localhostPort = '';
	nickname = '';

	playerStats: [number, number, number];

	gameName = 'gta5';

	lastServer: HistoryServer;
	HistoryServerStatus = HistoryServerStatus;

	topServer: Server;

	constructor(
		private tweetService: TweetService,
		private gameService: GameService,
		private discourseService: DiscourseService,
		private domSanitizer: DomSanitizer,
		private serversService: ServersService,
		public changeDetectorRef: ChangeDetectorRef,
		@Inject(L10N_LOCALE) public locale: L10nLocale,
	) {
		discourseService.signinChange.subscribe((user) => {
			this.currentAccount = user;

			this.loadTopServer();
		});

		this.serviceMessage = cachedServiceMessage;
		this.welcomeMessage = cachedWelcomeMessage;

		this.nickname = gameService.nickname;
		this.language = gameService.language;
		this.gameName = gameService.gameName;
		this.streamerMode = gameService.streamerMode;
		this.devMode = gameService.devMode;
		this.localhostPort = gameService.localhostPort;

		gameService.signinChange.subscribe((value) => {
			this.nickname = value.name;
		});
		gameService.nicknameChange.subscribe(value => this.nickname = value);
		gameService.languageChange.subscribe(value => this.language = value);
		gameService.streamerModeChange.subscribe(value => this.streamerMode = value);
		gameService.devModeChange.subscribe(value => this.devMode = value);
		gameService.localhostPortChange.subscribe(value => this.localhostPort = value);
		gameService.localServerChange.subscribe(value => {
			if (value.available) {
				this.localhost = true;
				this.localhostName = value.host;
				this.localhostPort = value.port;
			} else {
				this.localhost = false;
			}
		});
	}

	get topFlagCountry() {
		return this.topServer?.data?.vars?.locale?.split('-')[1]?.toLowerCase() ?? 'aq';
	}

	ngOnInit() {
		this.brandingName = this.gameService.brandingName;
		this.currentAccount = this.discourseService.currentUser;

		this.fetchTweets();
		this.fetchWelcome();
		this.fetchPlayerStats();
		this.fetchServiceMessage();
		this.updateStatus();
		this.statusInterval = this.startStatusCheckerLoop();

		this.loadLastServer();
		this.loadTopServer();
	}

	ngOnDestroy() {
		clearInterval(this.statusInterval);
	}

	async loadTopServer() {
		if (this.currentAccount) {
			const pinConfig = await this.serversService.loadPinConfig();

			this.topServer = await this.serversService.getServer(pinConfig.noAdServerId);
		} else {
			this.topServer = await this.serversService.getTopServer();
		}

		this.changeDetectorRef.markForCheck();
	}

	async loadLastServer() {
		const serverHistory = this.gameService.getServerHistory();

		const lastServer = serverHistory[serverHistory.length - 1];

		if (lastServer) {
			this.lastServer = {
				historyEntry: lastServer,
				server: null,
				sanitizedIcon: (lastServer.icon ? this.domSanitizer.bypassSecurityTrustUrl(lastServer.icon) : null),
				status: HistoryServerStatus.Loading,
			};

			const isAddressServer = this.lastServer.historyEntry.address.includes(':');
			const done = () => {
				this.changeDetectorRef.markForCheck();
			};

			if (!isAddressServer) {
				try {
					this.lastServer.server = await this.serversService.getServer(this.lastServer.historyEntry.address);
					this.lastServer.sanitizedIcon = this.lastServer.server.sanitizedUri;
					this.lastServer.status = HistoryServerStatus.Online;

					done();
					return;
				} catch (e) {
					this.lastServer.status = HistoryServerStatus.Offline;
				}
			}

			try {
				this.lastServer.server = await this.gameService.queryAddress(
					this.serversService.parseAddress(this.lastServer.historyEntry.address),
				);

				this.lastServer.sanitizedIcon = this.lastServer.server.sanitizedUri;

				// replace the server with a clean server-list server if need be
				this.serversService.tryGetJoinServer(this.lastServer.historyEntry.address).then(server => {
					if (server) {
						this.lastServer.server = server;
					}
				});

				this.lastServer.status = HistoryServerStatus.Online;
			} catch (e) {
				this.lastServer.status = HistoryServerStatus.Offline;
			}

			done();
		}
	}

	fetchServiceMessage() {
		window.fetch(`https://runtime.fivem.net/notice_${this.gameService.gameName}.html`)
			.then(async (res) => {
				if (res.ok) {
					this.serviceMessage = cachedServiceMessage = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
				}
			});
	}

	updateStatus() {
		window.fetch('https://status.cfx.re/api/v2/status.json')
			.then(async (res) => {
				const status = (await res.json());
				this.statusMessage = status['status']['description'] || '';
				switch (status['status']['description']) {
					case 'All Systems Operational':
						this.statusLevel = 1;
						break;
					case 'Partial System Outage':
					case 'Minor Service Outage':
						this.statusLevel = 2;
						break;
					case 'Major Service Outage':
						this.statusLevel = 3;
						break;
					default:
						this.statusLevel = 0;
						break;
				}
			})
			.catch(a => {});
	}
	startStatusCheckerLoop() {
		return setInterval(() => { this.updateStatus(); }, 1000 * 20 );
	}

	fetchWelcome() {
		window.fetch((this.gameService.gameName === 'gta5') ?
			'https://runtime.fivem.net/welcome.html' :
			`https://runtime.fivem.net/welcome_${this.gameService.gameName}.html`)
			.then(async res => {
				if (res.ok) {
					this.welcomeMessage = cachedWelcomeMessage = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
				}
			});
	}

	fetchPlayerStats() {
		window.fetch('https://runtime.fivem.net/counts.json')
			.then(async (res) => {
				this.playerStats = (await res.json()).map((c) => (Math.floor(c) / 1000).toFixed(1));
			})
			.catch(() => {
				this.playerStats = [-1, -1, -1];
			});
	}

	fetchTweets() {
		this.tweetService
			.getTweets('https://runtime.fivem.net/tweets.json')
			.then(tweets => {
				this.officialTweets = tweets.filter(a => !a.rt_displayname);
				this.communityTweets = [
					...tweets.filter(a => a.rt_displayname),
					...this.communityTweets
				];

				this.communityTweets.sort((a: Tweet, b: Tweet) => b.date.valueOf() - a.date.valueOf());
			});

		this.tweetService
			.getActivityTweets(
				this.gameService.getServerHistory()
					.filter(s => s.vars && s.vars.activitypubFeed)
					.map(s => s.vars.activitypubFeed))
			.subscribe(tweet => {
				this.communityTweets = [
					tweet,
					...this.communityTweets
				];

				this.communityTweets.sort((a: Tweet, b: Tweet) => b.date.valueOf() - a.date.valueOf());
			});
	}

	toggleAuthModal() {
		this.discourseService.openAuthModal();
	}

	clickContent(event: MouseEvent) {
		const srcElement = event.srcElement as HTMLElement;

		if (srcElement.localName === 'a') {
			this.gameService.openUrl(srcElement.getAttribute('href'));

			event.preventDefault();
			event.stopPropagation();
		}
	}

	openLink(link: string) {
		this.gameService.openUrl(link);
	}

	serviceMessageClick(event) {
		event.preventDefault();
		event.stopPropagation();

		if (event.target.localName === 'a') {
			const link = event.target.getAttribute('href');

			this.openLink(link);
		}

		return false;
	}

	connectToLocal() {
		(<any>window).invokeNative('connectTo', (this.localhostName || '127.0.0.1') + ':' + (this.localhostPort || '30120'));
	}

	attemptConnectTo(entry: HistoryServer) {
		if (!entry.server) {
			return;
		}

		this.gameService.connectTo(entry.server, entry.historyEntry.address);
	}

	async linkAccount() {
		const url = await this.discourseService.generateAuthURL();
		this.gameService.openUrl(url);
	}

	tweetTrack(idx: number, tweet: Tweet) {
		return tweet.id;
	}
}
