import { Component, OnInit, Inject, OnDestroy, ChangeDetectorRef, ɵSafeHtml } from '@angular/core';

import { Tweet, TweetService } from './tweet.service';

import { GameService } from '../game.service';
import { DiscourseService, DiscourseUser } from '../discourse.service';

import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { ServersService, HistoryServer, HistoryServerStatus } from 'app/servers/servers.service';
import { L10N_LOCALE, L10nLocale } from 'angular-l10n';

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

	brandingName: string;
	language = '';

	streamerMode = false;
	devMode = false;
	localhostPort = '';
	nickname = '';

	playerStats: [number, number];

	gameName = 'gta5';

	lastServer: HistoryServer;
	HistoryServerStatus = HistoryServerStatus;

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
	}

	ngOnInit() {
		this.brandingName = this.gameService.brandingName;
		this.currentAccount = this.discourseService.currentUser;

		this.fetchTweets();
		this.fetchWelcome();
		this.fetchPlayerStats();
		this.fetchServiceMessage();

		this.loadLastServer();
	}

	ngOnDestroy() {}

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

			if (!isAddressServer) {
				try {
					this.lastServer.server = await this.serversService.getServer(this.lastServer.historyEntry.address);
					this.lastServer.status = HistoryServerStatus.Online;
				} catch (e) {
					this.lastServer.status = HistoryServerStatus.Offline;
				}
			} else {
				try {
					this.lastServer.server = await this.gameService.queryAddress(
						this.serversService.parseAddress(this.lastServer.historyEntry.address),
					);

					this.lastServer.status = HistoryServerStatus.Online;
				} catch (e) {
					this.lastServer.status = HistoryServerStatus.Offline;
				}
			}

			this.changeDetectorRef.markForCheck();
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
		(<any>window).invokeNative('connectTo', '127.0.0.1:' + (this.localhostPort || '30120'));
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
}
