import {Injectable, EventEmitter, NgZone, Inject} from '@angular/core';
import {DomSanitizer} from '@angular/platform-browser';

import {Server} from './servers/server';

import { environment } from '../environments/environment';
import { LocalStorage } from './local-storage';
import { Observable, BehaviorSubject } from 'rxjs';

export class ConnectStatus {
	public server: Server;
	public message: string;
	public count: number;
	public total: number;
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
	public parameters: {[key: string]: string};
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

@Injectable()
export abstract class GameService {
	connectFailed = new EventEmitter<[Server, string]>();
	connectStatus = new EventEmitter<ConnectStatus>();
	connectCard = new EventEmitter<ConnectCard>();
	connecting = new EventEmitter<Server>();

	errorMessage = new EventEmitter<string>();
	infoMessage = new EventEmitter<string>();

	devModeChange = new BehaviorSubject<boolean>(false);
	darkThemeChange = new BehaviorSubject<boolean>(false);
	nicknameChange = new BehaviorSubject<string>('');
	localhostPortChange = new BehaviorSubject<string>('');
	languageChange = new BehaviorSubject<string>('en');

	signinChange = new EventEmitter<Profile>();
	ownershipTicketChange = new EventEmitter<string>();
	computerNameChange = new EventEmitter<string>();
	authPayloadSet = new EventEmitter<string>();

	inMinMode = false;
	minmodeBlob: any = {};

	minModeChanged = new EventEmitter<boolean>();

	profile: Profile = null;

	convars: { [name: string]: ConvarWrapper } = {};

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
			default:
				return 'CitizenFX';
		}
	}

	get nickname(): string {
		return 'UnknownPlayer';
	}

	set nickname(name: string) {

	}

	get devMode(): boolean {
		return false;
	}

	set devMode(value: boolean) {

	}

	get darkTheme(): boolean {
		return false;
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

	abstract isMatchingServer(type: string, server: Server): boolean;

	abstract toggleListEntry(type: string, server: Server, isInList: boolean): void;

	getProfile(): Profile {
		return this.profile;
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

	protected invokeConnectFailed(server: Server, message: string) {
		this.connectFailed.emit([server, message]);
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

	protected invokeConnectStatus(server: Server, message: string, count: number, total: number) {
		this.connectStatus.emit({
			server:  server,
			message: message,
			count:   count,
			total:   total
		});
	}

	protected invokeConnectCard(server: Server, cardBlob: string) {
		this.connectCard.emit({
			server:  server,
			card:    cardBlob
		});
	}

	protected invokeNicknameChanged(name: string) {
		this.nicknameChange.next(name);
	}

	protected invokeDevModeChanged(value: boolean) {
		this.devModeChange.next(value);
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

	public setDiscourseIdentity(token: string, clientId: string) {

	}

	public submitCardResponse(data: any) {

	}
}

export class ServerHistoryEntry {
	address: string;
	title: string;
	hostname: string;
	time: Date;
	icon: string;
	token: string;
}

@Injectable()
export class CfxGameService extends GameService {
	private _devMode = false;
	private _darkTheme = false;

	private lastServer: Server;

	private pingList: { [addr: string]: Server } = {};

	private pingListEvents: [string, number][] = [];

	private favorites: string[] = [];

	private history: string[] = [];

	private realNickname: string;
	
	private _localhostPort: string;

	private _language: string;
	
	private inConnecting = false;

	constructor(private sanitizer: DomSanitizer, private zone: NgZone) {
		super();
	}

	init() {
		(<any>window).invokeNative('getFavorites', '');
		(<any>window).invokeNative('getConvars', '');
		(<any>window).invokeNative('getMinModeInfo', '');

		fetch('https://nui-internal/profiles/list').then(async response => {
			try {
				const json = <Profiles>await response.json();

				if (json.profiles && json.profiles.length > 0) {
					this.handleSignin(json.profiles[0]);
				}
			} catch (e) {}
		});

		this.zone.runOutsideAngular(() => {
			window.addEventListener('message', (event) => {
				switch (event.data.type) {
					case 'connectFailed':
						this.zone.run(() => this.invokeConnectFailed(this.lastServer, event.data.message));
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
								this.lastServer, event.data.data.message, event.data.data.count, event.data.data.total));
						break;
					case 'connectCard':
						this.zone.run(() =>
							this.invokeConnectCard(
								this.lastServer, event.data.data.card));
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
						const convar = this.getConvarSubject(event.data.name);

						const convarItem = this.convars[event.data.name];
						convarItem.value = event.data.value;

						this.zone.run(() => convar.next(event.data.value));

						setTimeout(() => {
							this.ownershipTicketChange.emit(this.getConvarValue('cl_ownershipTicket'));
						}, 500);
						break;
					case 'setMinModeInfo':
						const enabled: boolean = event.data.enabled;
						const data = event.data.data;

						this.inMinMode = enabled;
						this.minmodeBlob = data;

						this.zone.run(() => {
							this.minModeChanged.emit(enabled);
						});

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
		});

		this.history = JSON.parse(localStorage.getItem('history')) || [];

		if (localStorage.getItem('nickOverride')) {
			//(<any>window).invokeNative('checkNickname', localStorage.getItem('nickOverride'));
			this.nickname = localStorage.getItem('nickOverride');
		}

		if (localStorage.getItem('devMode')) {
			this.devMode = localStorage.getItem('devMode') === 'yes';
		}

		if (localStorage.getItem('darkTheme')) {
			this.darkTheme = localStorage.getItem('darkTheme') === 'yes';
		}

		if (localStorage.getItem('localhostPort')) {
			this.localhostPort = localStorage.getItem('localhostPort');
		}

		if (localStorage.getItem('language')) {
			this.language = localStorage.getItem('language');
		}
		
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
					hostname: localStorage.getItem('lastServer'),
					icon: '',
					token: ''
				}
			);
		}
	}

	addServerHistory(entry: ServerHistoryEntry) {
		localStorage.setItem('lastServers', JSON.stringify(
			this.getServerHistory()
				.filter(a => a.address !== entry.address)
				.concat([ entry ])
		));
	}

	getServerHistory() {
		return (JSON.parse((localStorage.getItem('lastServers') || '[]')) as ServerHistoryEntry[]);
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

		(<any>window).invokeNative('checkNickname', name);
	}

	get darkTheme(): boolean {
		return this._darkTheme;
	}

	set darkTheme(value: boolean) {
		this._darkTheme = value;
		localStorage.setItem('darkTheme', value ? 'yes' : 'no');
		this.invokeDarkThemeChanged(value);
	}

	get devMode(): boolean {
		return this._devMode;
	}

	set devMode(value: boolean) {
		this._devMode = value;
		localStorage.setItem('devMode', value ? 'yes' : 'no');
		this.invokeDevModeChanged(value);
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

	connectTo(server: Server, enteredAddress?: string) {
		if (this.inConnecting) {
			return;
		}

		this.inConnecting = true;

		localStorage.setItem('lastServer', server.address);
		this.lastServer = server;

		this.addServerHistory({
			address: server.address,
			hostname: server.hostname,
			title: enteredAddress || '',
			time: new Date(),
			icon: server.iconUri || '',
			token: (server && server.data && server.data.vars) ? server.data.vars.sv_licenseKeyToken : ''
		});

		(<any>window).invokeNative('connectTo', this.getConnectAddress(server));

		// temporary, we hope
		this.history.push(server.address);
		this.saveHistory();
	}

	private getConnectAddress(server: Server): string {
		let connectAddress = server.address;

		if (server.connectEndPoints && server.connectEndPoints.length > 0) {
			connectAddress = server.connectEndPoints[Math.floor(Math.random() * server.connectEndPoints.length)];
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

	isMatchingServer(type: string, server: Server) {
		if (type == 'favorites') {
			return this.favorites.indexOf(server.address) >= 0;
		} else if (type == 'history') {
			return this.history.indexOf(server.address) >= 0;
		} else if (type == 'premium') {
			return server.data.vars && server.data.vars.premium;
		}

		return true;
	}

	toggleListEntry(list: string, server: Server, isInList: boolean) {
		if (this.isMatchingServer(list, server) !== isInList) {
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
		(<any>window).invokeNative('cancelDefer', '');
	}

	public setConvar(name: string, value: string) {
		(<any>window).invokeNative('setConvar', JSON.stringify({ name, value }));
	}

	lastQuery: string;

	queryAddress(address: [string, number]): Promise<Server> {
		const addrString = (address[0].match(/^[a-z0-9]{6,}$/))
			? `cfx.re/join/${address[0]}`
			: (address[0].indexOf('cfx.re') === -1)
				? address[0] + ':' + address[1]
				: address[0];

		const promise = new Promise<Server>((resolve, reject) => {
			const to = window.setTimeout(() => {
				if (addrString != this.lastQuery) {
					return;
				}

				reject(new Error('#DirectConnect_TimedOut'));

				window.removeEventListener('message', cb);
			}, 2500);

			const cb = (event) => {
				if (addrString != this.lastQuery) {
					window.removeEventListener('message', cb);
					return;
				}

				if (event.data.type == 'queryingFailed') {
					if (event.data.arg == addrString) {
						reject(new Error('#DirectConnect_Failed'));
						window.removeEventListener('message', cb);
						window.clearTimeout(to);
					}
				} else if (event.data.type == 'serverQueried') {
					resolve(Server.fromNative(this.sanitizer, event.data));
					window.removeEventListener('message', cb);
					window.clearTimeout(to);
				}
			};

			window.addEventListener('message', cb);
		});

		(<any>window).invokeNative('queryServer', addrString);
		this.lastQuery = addrString;

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
		(<any>window).invokeNative('submitCardResponse', JSON.stringify({ data }));
	}
}

@Injectable()
export class DummyGameService extends GameService {
	private _devMode = false;
	private _darkTheme = false;
	private _localhostPort = '';
	private _language = '';

	constructor(@Inject(LocalStorage) private localStorage: any) {
		super();

		if (this.localStorage.getItem('devMode')) {
			this._devMode = localStorage.getItem('devMode') === 'yes';
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
		this.minModeChanged.emit(false);
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
			this.invokeConnectStatus(server, 'hey!', 12, 12)

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
				hostname: 'Internal Test #1',
				icon: '',
				token: ''
			},
			{
				title: 'cfx-dev.fivem-2.internal',
				address: '127.0.0.1:30121',
				time: new Date(2018, 9, 1),
				hostname: 'Internal Test #2',
				icon: '',
				token: ''
			},
			{
				title: null,
				address: '51.15.201.219:30122',
				hostname: 'Hello, world!',
				time: new Date(),
				icon: '',
				token: 'ype00iiw33f7guwp_1:4a6aaf229eb26aa70d77f9d9e0039a6f28b3c9e3ad07cf307c1ce1ca6e071b42'
			}
		];
	}

	pingServers(servers: Server[]): Server[] {
		return servers;
	}

	isMatchingServer(type: string, server: Server): boolean {
		if (type == 'premium') {
			return server.data.vars && server.data.vars.premium;
		}

		return ((type !== 'history' && type !== 'favorites') || server.currentPlayers < 12);
	}

	toggleListEntry(list: string, server: Server, isInList: boolean) {
		console.log(`toggling ${list} entry ${server.address} (${isInList})`);
	}

	exitGame(): void {
		console.log('Exiting now');

		this.invokeError('You can\'t exit in a browser!');
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
	
	get devMode(): boolean {
		return this._devMode;
	}

	set devMode(value: boolean) {
		this._devMode = value;
		this.localStorage.setItem('devMode', value ? 'yes' : 'no');

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
