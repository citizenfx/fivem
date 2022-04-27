import { Injectable, EventEmitter, NgZone, Inject } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

import { Server, ServerHistoryEntry } from './servers/server';

import { environment } from '../environments/environment';
import { LocalStorage } from './local-storage';
import { Observable, BehaviorSubject, merge, of } from 'rxjs';
import * as query from 'query-string';
import { ActionSet, AdaptiveCard, SubmitAction, TextBlock, TextSize, Version } from 'adaptivecards';
import { L10nTranslationService } from 'angular-l10n';
import { master } from './servers/master';
import { map } from 'rxjs/operators';

export class ConnectStatus {
	public server: Server;
	public message: string;
	public count: number;
	public total: number;
	public cancelable: boolean;
}

export class ConnectCard {
	public server: Server;
	public card: string;
}

export class Profile {
	public name: string;
	public tile: string;
	public type: string;
	public identifier: number;
	public externalIdentifier: string;
	public parameters: { [key: string]: string };
	public signedIn: boolean;
}

export class Profiles {
	public profiles: Profile[];
}

class ConvarWrapper {
	observable: BehaviorSubject<string>;
	value: string;

	constructor(public name: string) {
		this.observable = new BehaviorSubject<string>('');
	}
}

export class LocalhostAvailability {
	available: boolean;
	host: string;
	port: string;

	constructor(available: boolean, host?: string, port?: string) {
		this.available = available;
		this.host = host ?? '';
		this.port = port ?? '';
	}
}

@Injectable()
export abstract class GameService {
	connectFailed = new EventEmitter<[Server, string, any]>();
	connectStatus = new EventEmitter<ConnectStatus>();
	connectCard = new EventEmitter<ConnectCard>();
	connecting = new EventEmitter<Server>();
	tryConnecting = new EventEmitter<string>();

	errorMessage = new EventEmitter<string>();
	infoMessage = new EventEmitter<string>();

	streamerModeChange = new BehaviorSubject<boolean>(false);
	devModeChange = new BehaviorSubject<boolean>(false);
	darkThemeChange = new BehaviorSubject<boolean>(true);
	requestOnBuildSwitchChange = new BehaviorSubject<boolean>(true);
	nicknameChange = new BehaviorSubject<string>('');
	localhostPortChange = new BehaviorSubject<string>('');
	localServerChange = new BehaviorSubject<LocalhostAvailability>(new LocalhostAvailability(false));
	languageChange = new BehaviorSubject<string>('en');

	signinChange = new EventEmitter<Profile>();
	ownershipTicketChange = new EventEmitter<string>();
	computerNameChange = new EventEmitter<string>();
	authPayloadSet = new EventEmitter<string>();

	inMinMode = false;
	inSwitchCL = false;
	minmodeBlob: any = {};

	minModeChanged = new BehaviorSubject<boolean>(false);

	profile: Profile = null;

	convars: { [name: string]: ConvarWrapper } = {};
	showConnectingOverlay: boolean;

	buildSwitchTimeout;
	buildSwitchUItimeouts = [];

	get systemLanguages(): string[] {
		return ['en-us'];
	}

	get gameName(): string {
		const targetGame = (window as any).nuiTargetGame;

		if (!targetGame) {
			return 'rdr3';
		}

		return targetGame;
	}

	get brandingName(): string {
		switch (this.gameName) {
			case 'rdr3':
				return 'RedM';
			case 'launcher':
				return 'Cfx.re';
			case 'gta5':
				return 'FiveM';
			case 'ny':
				return 'LibertyM';
			default:
				return 'CitizenFX';
		}
	}

	get nickname(): string {
		return 'UnknownPlayer';
	}

	set nickname(name: string) {

	}

	get streamerMode(): boolean {
		return false;
	}

	set streamerMode(value: boolean) {

	}

	get devMode(): boolean {
		return false;
	}

	set devMode(value: boolean) {

	}

	get requestOnBuildSwitch(): boolean {
		return true;
	}

	set requestOnBuildSwitch(value: boolean) {

	}

	get darkTheme(): boolean {
		return true;
	}

	set darkTheme(value: boolean) {

	}

	get localhostPort(): string {
		return '30120';
	}

	set localhostPort(name: string) {

	}

	get language(): string {
		return 'en';
	}

	set language(lang: string) {

	}

	abstract init(): void;

	abstract connectTo(server: Server, enteredAddress?: string): void;

	abstract pingServers(servers: Server[]): Server[];

	abstract isMatchingServer(type: string, server: master.IServer): boolean;

	abstract toggleListEntry(type: string, server: Server, isInList: boolean): void;

    async selectFile(key: string): Promise<string> {
        throw new Error('not on web');
    }

	async updateProfiles(withRockstar?: boolean) {}

	sayHello() {}

	getProfile(): Profile {
		return this.profile;
	}

	hasProfiles(): Observable<boolean> {
		return of(false);
	}

	getProfileString(): Observable<string> {
		return of('');
	}

	abstract getServerHistory(): ServerHistoryEntry[];

	handleSignin(profile: Profile): void {
		this.profile = profile;

		this.signinChange.emit(profile);
	}

	queryAddress(address: [string, number]): Promise<Server> {
		return new Promise<Server>((resolve, reject) => setTimeout(() => reject(new Error("Querying isn't supported in GameService.")), 2500));
	}

	exitGame(): void {

	}

	cancelNativeConnect(): void {

	}

	openUrl(url: string): void {
		const win = window.open(url, '_blank');
		win.focus();
	}

	protected invokeConnectFailed(server: Server, message: string, extra?: any) {
		this.connectFailed.emit([server, message, extra || {}]);
	}

	public invokeError(message: string) {
		this.errorMessage.emit(message);
	}

	public invokeInformational(message: string) {
		this.infoMessage.emit(message);
	}

	protected invokeConnecting(server: Server) {
		this.connecting.emit(server);
	}

	protected invokeConnectStatus(server: Server, message: string, count: number, total: number, cancelable: boolean) {
		this.connectStatus.emit({
			server: server,
			message: message,
			count: count,
			total: total,
            cancelable: cancelable
		});
	}

	protected invokeConnectCard(server: Server, cardBlob: string) {
		this.connectCard.emit({
			server: server,
			card: cardBlob
		});
	}

	protected invokeNicknameChanged(name: string) {
		this.nicknameChange.next(name);
	}

	protected invokeStreamerModeChanged(value: boolean) {
		this.streamerModeChange.next(value);
	}

	protected invokeDevModeChanged(value: boolean) {
		this.devModeChange.next(value);
	}
	
	protected invokeRequestOnBuildSwitchChanged(value: boolean) {
		this.requestOnBuildSwitchChange.next(value);
	}

	protected invokeDarkThemeChanged(value: boolean) {
		this.darkThemeChange.next(value);
	}

	protected invokeLocalhostPortChanged(port: string) {
		this.localhostPortChange.next(port);
	}

	protected invokeLanguageChanged(lang: string) {
		this.languageChange.next(lang);
	}

	protected getConvarSubject(name: string) {
		if (!this.convars[name]) {
			this.convars[name] = new ConvarWrapper(name);
		}

		return this.convars[name].observable;
	}

	public getConvar(name: string): Observable<string> {
		return this.getConvarSubject(name);
	}

	public getConvarValue(name: string) {
		if (!this.convars[name]) {
			this.convars[name] = new ConvarWrapper(name);
		}

		return this.convars[name].value;
	}

	public setConvar(name: string, value: string) {

	}

	public setArchivedConvar(name: string, value: string) {

	}

	public setDiscourseIdentity(token: string, clientId: string) {

	}

	public submitCardResponse(data: any) {

	}
}

@Injectable()
export class CfxGameService extends GameService {
	private _streamerMode = false;
	private _devMode = false;
	private _requestOnBuildSwitch = true;
	private _darkTheme = true;

	private profileListChange = new BehaviorSubject<boolean>(false);

	private lastServer: Server;

	private ownershipTicket = '';

	private pingList: { [addr: string]: Server } = {};

	private pingListEvents: [string, number][] = [];

	private favorites: string[] = [];

	private history: string[] = [];

	private realNickname: string;

	private _localhostPort: string;

	private _language: string;

	private inConnecting = false;

	private serverHistoryEntry: ServerHistoryEntry = null;
	private connectNonce = '';

	private profileList: Record<string, string> = {};
	card: boolean;

	constructor(private sanitizer: DomSanitizer, private zone: NgZone, private translation: L10nTranslationService) {
		super();
	}

	init() {
		(<any>window).invokeNative('backfillEnable', '');
		(<any>window).invokeNative('getFavorites', '');
		(<any>window).invokeNative('getConvars', '');

		fetch('https://nui-internal/profiles/list').then(async response => {
			try {
				const json = <Profiles>await response.json();

				if (json.profiles && json.profiles.length > 0) {
					this.profileList[json.profiles[0].externalIdentifier] = json.profiles[0].name;
					this.profileListChange.next(true);

					this.handleSignin(json.profiles[0]);
				}
			} catch (e) { }
		});

		const handleSetConvar = (name: string, value: string) => {
			const convar = this.getConvarSubject(name);

			const convarItem = this.convars[name];
			convarItem.value = value;

			convar.next(value);

			if (name === 'cl_ownershipTicket' && value !== '') {
				setTimeout(() => {
					if (this.ownershipTicket !== this.getConvarValue('cl_ownershipTicket')) {
						this.ownershipTicket = this.getConvarValue('cl_ownershipTicket');

						this.ownershipTicketChange.emit(this.getConvarValue('cl_ownershipTicket'));
					}
				}, 500);
			}
		};

		this.zone.runOutsideAngular(() => {
			window.addEventListener('message', (event) => {
				switch (event.data.type) {
					case 'exitGameplay':
						document.body.style.visibility = 'visible';
						break;
                    case 'fileDialogResult':
                        this.zone.run(() => this.invokeFileDialogResult(event.data.dialogKey, event.data.result));
                        break;
					case 'connectFailed':
						this.zone.run(() => this.invokeConnectFailed(this.lastServer, event.data.message, event.data.extra));
						break;
					case 'setWarningMessage':
						this.zone.run(() => this.invokeError(event.data.message));
						break;
					case 'authPayload':
						this.zone.run(() => this.invokeAuthPayload(event.data.data));
						break;
					case 'connecting':
						this.zone.run(() => this.invokeConnecting(this.lastServer));
						break;
					case 'connectStatus':
						this.zone.run(() =>
							this.invokeConnectStatus(
								this.lastServer, event.data.data.message, event.data.data.count, event.data.data.total, event.data.data.cancelable));
						break;
					case 'connectCard':
						this.zone.run(() =>
							this.invokeConnectCard(
								this.lastServer, event.data.data.card));
						break;
					case 'connectBuildSwitchRequest':
						this.zone.run(() =>
							this.invokeBuildSwitchRequest(
								this.lastServer, event.data.data.build));
						break;
					case 'connectBuildSwitch':
						this.zone.run(() =>
							this.invokeBuildSwitch(
								this.lastServer, event.data.data.title, event.data.data.content));
						break;
					case 'serverAdd':
						if (event.data.addr in this.pingList) {
							this.pingListEvents.push([event.data.addr, event.data.ping]);
						}
						break;
					case 'setComputerName':
						this.computerNameChange.emit(event.data.data);
						break;
					case 'getFavorites':
						this.zone.run(() => this.favorites = event.data.list);
						break;
					case 'addToHistory':
						this.history.push(event.data.address);
						this.saveHistory();
						break;
					case 'convarSet':
						this.zone.run(() => {
							handleSetConvar(event.data.name, event.data.value);
						});
						break;
					case 'convarsSet':
						this.zone.run(() => {
							for (const { key, value } of event.data.vars) {
								handleSetConvar(key, value);
							}
						});
						break;
					case 'setMinModeInfo':
						const enabled: boolean = event.data.enabled;
						const data = event.data.data;

						this.inMinMode = enabled;
						this.minmodeBlob = data;

						this.zone.run(() => {
							this.minModeChanged.next(enabled);
						});

						break;
					case 'setSwitchCl':
						this.zone.run(() => {
							this.inSwitchCL = event.data.enabled;
						});
						break;
					case 'connectTo':
						const address: string = event.data.hostnameStr;
						const connectParams = query.parse(event.data.connectParams);

						this.updateNickname();

						if (!this.inConnecting) {
							if ('streamerMode' in connectParams) {
								const streamerMode = ['true', '1'].includes(<string>connectParams.streamerMode);
								this._streamerMode = streamerMode;
								this.invokeStreamerModeChanged(streamerMode);
							}

							if ('switchcl' in connectParams) {
								const switchCL = ['true', 1].includes(<string>connectParams.switchcl);
								this.inSwitchCL = switchCL;
							}

							this.zone.run(() => {
								this.inConnecting = true;

								this.tryConnecting.emit(address);
							});
						}

						break;
					case 'backfillServerInfo':
						this.backfillServerInfo(event.data.data);
						break;
				}
			});

			window.setInterval(() => {
				if (this.pingListEvents.length > 0) {
					this.zone.run(() => {
						const ple = this.pingListEvents;

						for (const [serverId, ping] of ple) {
							this.pingList[serverId].updatePing(ping);
						}
					});
				}

				this.pingListEvents = [];
			}, 250);

			const requestLocalhost = async () => {
				try {
					const localhostServer = await this.queryAddress(['localhost_sentinel', parseInt(this.localhostPort, 10) || 30120]);

					if (localhostServer) {
						this.localServerChange.next(
							new LocalhostAvailability(
								true,
								localhostServer.data?.address ?? 'localhost',
								localhostServer.data?.port ?? (this.localhostPort || '30120')
							)
						);
						this.devMode = true;
					} else {
						this.devMode = false;

						this.localServerChange.next(new LocalhostAvailability(false));
					}
				} catch {
					this.devMode = false;

					this.localServerChange.next(new LocalhostAvailability(false));
				}
			};
			requestLocalhost();

			window.setInterval(requestLocalhost, 5000);
		});

		this.history = JSON.parse(localStorage.getItem('history')) || [];

		if (localStorage.getItem('nickOverride')) {
			this.nickname = localStorage.getItem('nickOverride');
		}

		if (localStorage.getItem('requestOnBuildSwitch')) {
			this.requestOnBuildSwitch = localStorage.getItem('requestOnBuildSwitch') !== 'no';
		}

		if (localStorage.getItem('darkThemeNew')) {
			this.darkTheme = localStorage.getItem('darkThemeNew') !== 'no';
		}

		if (localStorage.getItem('localhostPort')) {
			this.localhostPort = localStorage.getItem('localhostPort');
		}

		if (localStorage.getItem('language')) {
			const lang = localStorage.getItem('language');
			(<any>window).invokeNative('setLocale', lang);
			this.language = lang;
		}

		this.getConvar('ui_streamerMode').subscribe(value => {
			this._streamerMode = value === 'true';
			this.invokeStreamerModeChanged(value === 'true');
		});

		this.connecting.subscribe(server => {
			this.inConnecting = false;
		});

		(<any>window).invokeNative('loadWarning', '');

		// migrate lastServer -> lastServers
		if (!localStorage.getItem('lastServers') && localStorage.getItem('lastServer')) {
			this.addServerHistory(
				{
					address: localStorage.getItem('lastServer'),
					time: new Date(),
					title: '',
					vars: {},
					hostname: localStorage.getItem('lastServer'),
					icon: '',
					token: '',
					rawIcon: ''
				}
			);
		}
	}

    private fileSelectReqs: { [key: string]: (result: string) => void } = {};

    private invokeFileDialogResult(key: string, result?: string) {
        if (this.fileSelectReqs[key]) {
            this.fileSelectReqs[key](result ?? '');
        }
    }

    async selectFile(key: string): Promise<string> {
        return new Promise<string>((resolve) => {
            (window as any).invokeNative('openFileDialog', key);

            this.fileSelectReqs[key] = resolve;
        });
    }

	protected invokeBuildSwitchRequest(server: Server, build: number) {
		this.card = true;

		if (this.requestOnBuildSwitch) {
			const presentCard = (seconds: number) => {
				if (!this.card) {
					return;
				}
	
				const card = new AdaptiveCard();
				card.version = new Version(1, 0);
	
				let gameBrand = 'CitizenFX';
	
				if (this.gameName === 'rdr3') {
					gameBrand = 'RedM';
				} else if (this.gameName === 'gta5') {
					gameBrand = 'FiveM';
				}
	
				const heading = new TextBlock(this.translation.translate('#BuildSwitch_Heading', { build, gameBrand }));
				heading.size = TextSize.ExtraLarge;
				card.addItem(heading);
	
				const body = new TextBlock(this.translation.translate('#BuildSwitch_Body', { build, seconds }));
				body.wrap = true;
				card.addItem(body);
	
				const cancelAction = new SubmitAction();
				cancelAction.data = { action: 'cancel' };
				cancelAction.title = this.translation.translate('#BuildSwitch_Cancel');
	
				const okAction = new SubmitAction();
				okAction.data = { action: 'ok' };
				okAction.style = 'positive';
				okAction.title = this.translation.translate('#BuildSwitch_OK', { seconds });
	
				const actionSet = new ActionSet();
				actionSet.addAction(cancelAction);
				actionSet.addAction(okAction);
				card.addItem(actionSet);
	
				this.connectCard.emit({
					server: server,
					card: JSON.stringify(card.toJSON())
				});
			};
	
			this.buildSwitchUItimeouts.forEach(clearTimeout);
			this.buildSwitchUItimeouts.length = 0;
	
			for (let i = 0; i < 10; i++) {
				const msec = (10 - i) * 1000;
				const sec = i;
	
				this.buildSwitchUItimeouts.push(setTimeout(() => presentCard(sec), msec));
			}
	
			if (this.buildSwitchTimeout) {
				clearTimeout(this.buildSwitchTimeout);
			}
			this.buildSwitchTimeout = setTimeout(() => {
				if (this.card) {
					this.submitCardResponse({
						action: 'ok'
					});
				}
			}, 10000);
		}
		else {
			this.submitCardResponse({
				action: 'ok'
			});
		}
	}

	protected invokeBuildSwitch(server: Server, title: string, content: string) {
		const card = new AdaptiveCard();
		card.version = new Version(1, 0);

		const heading = new TextBlock(title);
		heading.size = TextSize.ExtraLarge;
		card.addItem(heading);

		const body = new TextBlock(content);
		body.wrap = true;
		card.addItem(body);

		const cancelAction = new SubmitAction();
		cancelAction.data = { action: 'cancel' };
		cancelAction.title = this.translation.translate('#No');

		const okAction = new SubmitAction();
		okAction.data = { action: 'ok' };
		okAction.style = 'positive';
		okAction.title = this.translation.translate('#Yes');

		const actionSet = new ActionSet();
		actionSet.addAction(cancelAction);
		actionSet.addAction(okAction);
		card.addItem(actionSet);

		this.connectCard.emit({
			server: server,
			card: JSON.stringify(card.toJSON())
		});
	}

	get systemLanguages(): string[] {
		return (window as any).nuiSystemLanguages || ['en-us'];
	}

	async addServerHistory(entry: ServerHistoryEntry) {
		try {
			const resp = await fetch(entry.icon);

			if (resp.ok) {
				const blob = await resp.blob();
				const bitmap = (await createImageBitmap(blob, {
					resizeQuality: 'medium',
					resizeWidth: 16,
					resizeHeight: 16
				})) as ImageBitmap;

				const canvas = Object.assign(document.createElement('canvas'), {
					width: bitmap.width,
					height: bitmap.height
				});

				const cxt = canvas.getContext('2d');
				cxt.drawImage(bitmap, 0, 0);

				const outBlob = await new Promise<Blob>(resolve => canvas.toBlob(resolve, 'image/png'));

				entry.rawIcon = await new Promise(resolve => {
					const r = new FileReader();
					r.onload = () => resolve(r.result as string);
					r.readAsDataURL(outBlob);
				});
			}
		} catch (e) {
		}

		function canonicalize(address: string) {
			return address.replace(/^cfx.re\/join\//, '');
		}

		const lastServers = JSON.stringify(
			this.getServerHistory()
				.filter(a => canonicalize(a.address) !== canonicalize(entry.address))
				.concat([entry]));

		entry.address = canonicalize(entry.address);

		localStorage.setItem('lastServers', lastServers);
		(<any>window).invokeNative('setLastServers', lastServers);
	}

	getServerHistory() {
		return (JSON.parse((localStorage.getItem('lastServers') || '[]')) as ServerHistoryEntry[]).map(a => ({
			...a,
			time: new Date(a.time)
		}));
	}

	invokeAuthPayload(data: string) {
		this.authPayloadSet.emit(data);
	}

	get nickname(): string {
		return this.realNickname;
	}

	set nickname(name: string) {
		this.realNickname = name;
		localStorage.setItem('nickOverride', name);
		this.invokeNicknameChanged(name);

		this.updateNickname();
	}

	updateNickname() {
		if (this.realNickname && this.realNickname !== '') {
			(<any>window).invokeNative('checkNickname', this.realNickname);
		}
	}
	
	get requestOnBuildSwitch(): boolean {
		return this._requestOnBuildSwitch;
	}

	set requestOnBuildSwitch(value: boolean) {
		this._requestOnBuildSwitch = value;
		localStorage.setItem('requestOnBuildSwitch', value ? 'yes' : 'no');
		this.invokeRequestOnBuildSwitchChanged(value);
	}

	get darkTheme(): boolean {
		return this._darkTheme;
	}

	set darkTheme(value: boolean) {
		this._darkTheme = value;
		localStorage.setItem('darkThemeNew', value ? 'yes' : 'no');
		this.invokeDarkThemeChanged(value);
	}

	get streamerMode(): boolean {
		return this._streamerMode;
	}

	set streamerMode(value: boolean) {
		this._streamerMode = value;
		this.setArchivedConvar('ui_streamerMode', value ? 'true' : 'false');
		this.invokeStreamerModeChanged(value);
	}

	get devMode(): boolean {
		return this._devMode;
	}

	set devMode(value: boolean) {
		const oldValue = this._devMode;
		this._devMode = value;

		if (oldValue !== value) {
			this.invokeDevModeChanged(value);
		}
	}

	get localhostPort(): string {
		return this._localhostPort;
	}

	set localhostPort(port: string) {
		this._localhostPort = port;
		localStorage.setItem('localhostPort', port);
		this.invokeLocalhostPortChanged(port);
	}

	get language(): string {
		return this._language;
	}

	set language(lang: string) {
		this._language = lang;
		localStorage.setItem('language', lang);
		this.invokeLanguageChanged(lang);
	}

	private saveHistory() {
		localStorage.setItem('history', JSON.stringify(this.history));
	}

	sayHello() {
		(<any>window).invokeNative('getMinModeInfo', '');
	}

	private async backfillServerInfo(eventData: any) {
		if (this.connectNonce === '' || eventData.nonce !== this.connectNonce) {
			(<any>window).invokeNative('backfillDone', '');
			return;
		}

		const newHistoryEntry: ServerHistoryEntry = {
			...this.serverHistoryEntry,
			...eventData.server
		};

		await this.addServerHistory(newHistoryEntry);
		this.connectNonce = '';
		this.serverHistoryEntry = null;

		(<any>window).invokeNative('backfillDone', '');
	}

	async connectTo(server: Server, enteredAddress?: string) {
		if (this.inConnecting) {
			return;
		}

		this.updateNickname();

		this.inConnecting = true;

		localStorage.setItem('lastServer', server.address);
		this.lastServer = server;

		const historyEntry: ServerHistoryEntry = {
			address: server.address,
			hostname: server.hostname.replace(/\^[0-9]/g, ''),
			title: enteredAddress || '',
			time: new Date(),
			icon: server.iconUri || '',
			rawIcon: '',
			vars: server.data?.vars || {},
			token: server.data?.vars?.sv_licenseKeyToken || '',
		};

		await this.addServerHistory(historyEntry);
		this.serverHistoryEntry = historyEntry;
		this.connectNonce = (new Date()).getTime().toString();

		(<any>window).invokeNative('connectTo', JSON.stringify([
			this.getConnectAddress(server, enteredAddress),
			this.connectNonce,
		]));

		// temporary, we hope
		this.history.push(server.address);
		this.saveHistory();
	}

	private getConnectAddress(server: Server, enteredAddress?: string): string {
		let connectAddress = server.address;

		if (server.connectEndPoints && server.connectEndPoints.length > 0 &&
			server.connectEndPoints[0] !== 'https://private-placeholder.cfx.re/') {
			connectAddress = server.connectEndPoints[Math.floor(Math.random() * server.connectEndPoints.length)];
		} else if (enteredAddress) {
			connectAddress = enteredAddress;
		}

		return connectAddress;
	}

	pingServers(servers: Server[]) {
		for (const server of servers) {
			this.pingList[server.address] = server;
		}

		(<any>window).invokeNative('pingServers', JSON.stringify(
			servers.map(a => {
				const address = this.getConnectAddress(a);
				return [address.split(':')[0], parseInt(address.split(':')[1]), a.currentPlayers]
			})
		));

		return servers;
	}

	isMatchingServer(type: string, server: master.IServer) {
		if (type === 'favorites') {
			return this.favorites.indexOf(server?.EndPoint) >= 0;
		} else if (type === 'history') {
			return this.history.indexOf(server?.EndPoint) >= 0;
		} else if (type === 'premium') {
			return !!server?.Data?.vars?.premium;
		}

		return true;
	}

	toggleListEntry(list: string, server: Server, isInList: boolean) {
		if (this.isMatchingServer(list, {
			EndPoint: server.address,
			Data: server.data
		}) !== isInList) {
			if (isInList) {
				if (list == 'favorites') {
					this.favorites.push(server.address);
				} else if (list == 'history') {
					this.history.push(server.address);

				}
			} else {
				if (list == 'favorites') {
					this.favorites = this.favorites.filter(a => a != server.address);
				} else if (list == 'history') {
					this.history = this.history.filter(a => a != server.address);
				}
			}
		}

		if (list == 'favorites') {
			(<any>window).invokeNative('saveFavorites', JSON.stringify(this.favorites))
		} else if (list == 'history') {
			this.saveHistory();
		}
	}

	cancelNativeConnect(): void {
        this.card = false;

		(<any>window).invokeNative('cancelDefer', '');
	}

	public setConvar(name: string, value: string) {
		(<any>window).invokeNative('setConvar', JSON.stringify({ name, value }));
	}

	public setArchivedConvar(name: string, value: string) {
		(<any>window).invokeNative('setArchivedConvar', JSON.stringify({ name, value }));
	}

	queryQueue: Set<{
		tries: string[],
		resolve: any,
		reject: any,
	}> = new Set();
	queuing = false;

	private async workQueryQueue() {
		if (this.queuing) {
			return;
		}

		this.queuing = true;

		queue: for (const queryItem of this.queryQueue) {
			const { tries, resolve, reject } = queryItem;

			let lastError: Error = null;

			for (const addrString of tries) {
				const promise = new Promise<Server>((queryResolve, queryReject) => {
                    let messageHandler: any = null;

					const timeoutTimer = window.setTimeout(() => {
						queryReject(new Error('#DirectConnect_TimedOut'));

						window.removeEventListener('message', messageHandler);
					}, 7500);

					messageHandler = (event) => {
						if (event.data.type === 'queryingFailed') {
							if (event.data.arg === addrString) {
								queryReject(new Error('#DirectConnect_Failed'));
								window.removeEventListener('message', messageHandler);
								window.clearTimeout(timeoutTimer);
							}
						} else if (event.data.type === 'serverQueried') {
                            if (event.data.queryCorrelation === addrString) {
                                queryResolve(Server.fromNative(this.sanitizer, event.data));
                                window.removeEventListener('message', messageHandler);
                                window.clearTimeout(timeoutTimer);
                            }
						}
					};

					window.addEventListener('message', messageHandler);
				});

				(<any>window).invokeNative('queryServer', addrString);

				try {
					resolve(await promise);
					this.queryQueue.delete(queryItem);
					continue queue;
				} catch (e) {
					lastError = e;
				}
			}

			reject(lastError);

			this.queryQueue.delete(queryItem);
		}

		this.queuing = false;
	}

	async queryAddress(address: [string, number]): Promise<Server> {
		const tries = [];

		if (address[0].match(/^[a-z0-9]{6,}$/) && address[0] !== 'localhost') {
			tries.push(`cfx.re/join/${address[0]}`);
		}

		tries.push((address[0].indexOf('cfx.re') === -1 && address[1])
			? address[0] + ':' + address[1]
			: address[0]);

		const promise = new Promise<Server>((resolve, reject) => {
			this.queryQueue.add({ tries, resolve, reject });
		});

		if (!this.queuing) {
			setTimeout(() => this.workQueryQueue(), 0);
		}

		return promise;
	}

	exitGame(): void {
		(<any>window).invokeNative('exit', '');
	}

	openUrl(url: string): void {
		(<any>window).invokeNative('openUrl', url);
	}

	setDiscourseIdentity(token: string, clientId: string) {
		(<any>window).invokeNative('setDiscourseIdentity', JSON.stringify({ token, clientId }));
	}

	public submitCardResponse(data: any) {
		this.card = false;

		(<any>window).invokeNative('submitCardResponse', JSON.stringify({ data }));
	}

	async updateProfiles(withRockstar?: boolean) {
		const r = await fetch('https://lambda.fivem.net/api/ticket/identities', {
			method: 'POST',
			body: `token=${this.ownershipTicket}&withRockstar=${withRockstar ?? false}`,
			headers: {
				'content-type': 'application/x-www-form-urlencoded'
			}
		});

		if (!r.ok) {
			return;
		}

		const j = await r.json();
		for (const identity of j.identities) {
			this.profileList[identity.id] = identity.username;
		}
		this.profileListChange.next(true);
	}

	hasProfiles() {
		return this.profileListChange.pipe(map(_ => Object.entries(this.profileList).length > 0));
	}

	getProfileString(): Observable<string> {
		return merge(
			this.streamerModeChange,
			this.profileListChange
		).pipe(map(_ => {
			if (this.streamerMode) {
				return Array(this.profileList.length)
					.fill('<i class="fas fa-user-secret"></i>&nbsp; &lt;HIDDEN&gt;')
					.join(',&nbsp;&nbsp;');
			}

			return Object.entries(this.profileList).map(([id, username]) => getIcon(id.split(':')[0]) + ' ' + htmlEscape(username)).join(',&nbsp;&nbsp;');

			function htmlEscape(unsafe: string) {
				return unsafe
					.replace(/&/g, '&amp;')
					.replace(/</g, '&lt;')
					.replace(/>/g, '&gt;')
					.replace(/"/g, '&quot;')
					.replace(/'/g, '&#039;');
			}

			function getIcon(type: string) {
				switch (type) {
					case 'steam':
						return '<i class="fab fa-steam"></i>';
					case 'discord':
						return '<i class="fab fa-discord"></i>';
					case 'xbl':
						return '<i class="fab fa-xbox"></i>';
					case 'ros':
						return `<svg viewBox="0 0 128 128" version="1.1" xmlns="http://www.w3.org/2000/svg">
	<path d="M126.7,82.9L99.6,82.9L95.3,55.9L79.5,82.7L76.5,82.7C74.7,79.6 74,75.1 74,72.3C74,67.7 74.3,63.2 74.3,57.3C74.3,49.5 72,45.4 65.9,43.9L65.9,43.7C78.8,41.9 84.7,33.3 84.7,21.3C84.7,4.2 73.3,0.5 58.4,0.5L18.3,0.5L1.3,80.9L22.6,80.9L28.8,51.6L43,51.6C50.6,51.6 53.7,55.3 53.7,62.4C53.7,67.8 53.1,72.1 53.1,76.2C53.1,77.7 53.4,81.3 54.5,82.7L69.9,99L56.6,127.5L85,110.6L106.2,126.9L102.3,100L126.7,82.9ZM49.4,36.6L32.4,36.6L36.5,17.2L52.3,17.2C57.9,17.2 63.8,18.7 63.8,25.5C63.8,34.2 57.1,36.6 49.4,36.6M85.4,105.9L65.6,117.7L74.7,98.3L63.7,86.7L81.8,86.7L93.2,67.4L96.3,86.9L114.3,86.9L98.1,98.2L101,118L85.4,105.9Z"/>
</svg>`;
					case 'fivem':
						return '<i class="fa fa-road"></i>';
				}

				return '';
			}
		}));
	}
}

@Injectable()
export class DummyGameService extends GameService {
	private _streamerMode = false;
	private _devMode = false;
	private _darkTheme = true;
	private _localhostPort = '';
	private _language = '';

	constructor(@Inject(LocalStorage) private localStorage: any) {
		super();

		if (this.localStorage.getItem('streamerMode')) {
			this._streamerMode = localStorage.getItem('streamerMode') === 'yes';
		}
	}

	init() {
		const profile = new Profile();
		profile.name = 'dummy';
		profile.externalIdentifier = 'dummy:1212';
		profile.signedIn = true;
		profile.type = 'dummy';
		profile.tile = '';
		profile.identifier = 0;
		profile.parameters = {};

		this.handleSignin(profile);
		this.minModeChanged.next(false);
	}

	connectTo(server: Server, enteredAddress?: string) {
		if (environment.web) {
			const ifr = document.createElement('iframe');
			ifr.src = `fivem://connect/${server.address}`;
			ifr.style.display = 'none';
			document.body.appendChild(ifr);

			this.invokeConnectFailed(server, 'If it is installed, FiveM should have launched. ' +
				'If it didn\'t, just join the following IP: ' + server.address);
			return;
		}

		console.log('faking connection to ' + server.address);

		this.invokeConnecting(server);

		setTimeout(() => {
			this.invokeConnectStatus(server, 'hey!', 12, 12, false)

			setTimeout(() => {
				this.invokeConnectFailed(server, 'Sorry, we\'re closed. :(');
			}, 500);
		}, 500);
	}

	getServerHistory() {
		return [
			{
				title: 'cfx-dev.fivem.internal',
				address: '127.0.0.1:30120',
				time: new Date(2018, 8, 1),
				vars: {},
				hostname: 'Internal Test #1',
				icon: '',
				token: '',
				rawIcon: ''
			},
			{
				title: 'cfx-dev.fivem-2.internal',
				address: '127.0.0.1:30121',
				time: new Date(2018, 9, 1),
				vars: {},
				hostname: 'Internal Test #2',
				icon: '',
				token: '',
				rawIcon: ''
			},
			{
				title: null,
				address: '51.15.201.219:30122',
				hostname: 'Hello, world!',
				time: new Date(),
				vars: {},
				icon: '',
				token: 'ype00iiw33f7guwp_1:4a6aaf229eb26aa70d77f9d9e0039a6f28b3c9e3ad07cf307c1ce1ca6e071b42',
				rawIcon: ''
			}
		];
	}

	pingServers(servers: Server[]): Server[] {
		return servers;
	}

	isMatchingServer(type: string, server: master.IServer): boolean {
		if (type === 'premium') {
			return server.Data.vars && (server.Data.vars.premium ? true : false);
		}

		return ((type !== 'history' && type !== 'favorites') || server.Data.clients < 12);
	}

	toggleListEntry(list: string, server: Server, isInList: boolean) {
		console.log(`toggling ${list} entry ${server.address} (${isInList})`);
	}

	exitGame(): void {
		console.log('Exiting now');

		this.invokeError('You can\'t exit in a browser!');
	}

	get gameName(): string {
		if (environment.web) {
			return location.hostname.indexOf('redm.gg') >= 0 ? 'rdr3' : 'gta5';
		}

		return 'gta5';
	}

	get nickname(): string {
		return this.localStorage.getItem('nickOverride') || 'UnknownPlayer';
	}

	set nickname(name: string) {
		this.localStorage.setItem('nickOverride', name);

		this.invokeNicknameChanged(name);
	}

	get localhostPort(): string {
		return this._localhostPort;
	}

	set localhostPort(port: string) {
		this.localStorage.setItem('localhostPort', port);

		this.invokeLocalhostPortChanged(port);
	}

	get streamerMode(): boolean {
		return this._streamerMode;
	}

	set streamerMode(value: boolean) {
		this._streamerMode = value;
		this.localStorage.setItem('streamerMode', value ? 'yes' : 'no');

		this.invokeStreamerModeChanged(value);
	}

	get devMode(): boolean {
		return this._devMode;
	}

	set devMode(value: boolean) {
		this._devMode = value;

		this.invokeDevModeChanged(value);
	}

	get language(): string {
		return this.localStorage.getItem('language') || navigator.language;
	}

	set language(lang: string) {
		this.localStorage.setItem('language', lang);

		this.invokeLanguageChanged(lang);
	}
}
