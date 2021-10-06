import { Injectable, NgZone, Inject, PLATFORM_ID } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer, SafeUrl } from '@angular/platform-browser';

import {Observable, of, Subject} from 'rxjs';

import { concat, from, BehaviorSubject } from 'rxjs';

import 'rxjs/add/observable/fromPromise';
import 'rxjs/add/operator/bufferTime';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/mergeMap';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

import { Server, ServerHistoryEntry } from './server';
import { PinConfig } from './pins';

import { master } from './master';
import { isPlatformBrowser } from '@angular/common';
import { GameService } from '../game.service';
import { FilterRequest } from './filter-request';
import { catchError, timeout } from 'rxjs/operators';
import { SearchAutocompleteIndex } from './filters.service';

const serversWorker = new Worker(new URL('./workers/servers.worker', import.meta.url), { type: 'module' });

export enum HistoryServerStatus {
	Loading,
	Online,
	Offline,
};

export interface HistoryServer {
	historyEntry: ServerHistoryEntry,
	server?: Server,
	sanitizedIcon: SafeUrl,
	status: HistoryServerStatus
};

class ServerCacheEntry {
	public server: Server;
	public lastTime: Date;

	public constructor(server: Server) {
		this.lastTime = new Date();
		this.server = server;
	}

	public isValid() {
		return (new Date().getTime() - this.lastTime.getTime()) < 15000;
	}
}

export interface TagIndex { [key: string]: number };

@Injectable()
export class ServersService {
	private requestEvent: Subject<string>;
	private serversEvent: Subject<master.IServer>;

	private worker: Worker;

	public tagUpdate: Subject<[TagIndex, TagIndex]> = new BehaviorSubject([{}, {}]);
	public autoCompleteUpdate: Subject<SearchAutocompleteIndex> = new BehaviorSubject({});

	public servers = new Map<string, master.IServer>();
	public serversLoadedUpdate: Subject<boolean> = new BehaviorSubject(false);
	private serverCacheMaterialized: { [addr: string]: Server } = {};

	private serverCache: { [addr: string]: ServerCacheEntry } = {};

	private onSortCB: ((servers: string[]) => void)[] = [];

	private topServer: Server = undefined;

	private firstLoad = false;

	constructor(private httpClient: HttpClient, private domSanitizer: DomSanitizer, private zone: NgZone,
		private gameService: GameService, @Inject(PLATFORM_ID) private platformId: any) {
		this.requestEvent = new Subject<string>();

		this.serversEvent = new Subject<master.IServer>();

		// only enable the worker if streams are supported
		if (typeof window !== 'undefined' && window.hasOwnProperty('Response') && Response.prototype.hasOwnProperty('body')) {
			this.worker = serversWorker;
			zone.runOutsideAngular(() => {
				this.worker.addEventListener('message', (event) => {
					if (event.data.type === 'addServers') {
						for (const server of (event.data.servers as master.IServer[])) {
							if (this.matchesGame(server)) {
								if (server && server.Data) {
									this.servers.set(server.EndPoint, server);
									this.serversEvent.next(server);
								}
							}
						}

						if (!this.firstLoad) {
							this.serversLoadedUpdate.next(false);
							this.firstLoad = true;
						}
					} else if (event.data.type === 'serversDone') {
						this.serversLoadedUpdate.next(true);
						this.firstLoad = false;
					} else if (event.data.type === 'sortedServers') {
						if (this.onSortCB.length) {
							zone.run(() => {
								this.onSortCB[0](event.data.servers);
								this.onSortCB.shift();
							});
						}
					} else if (event.data.type === 'updateTags') {
						this.tagUpdate.next(event.data.data);
					} else if (event.data.type === 'updateAutoComplete') {
						this.autoCompleteUpdate.next(event.data.data);
					} else {
						console.log('[servers] worker message rcv', event);
					}
				});
			});

			this.requestEvent
				.subscribe(url => {
					this.worker.postMessage({ type: 'queryServers', url: url + `streamRedir/` });
				});
		}

		this.gameService.tryConnecting.subscribe(async (serverHost: string) => {
			let server: Server = null;

			this.gameService.connecting.emit(Server.fromObject(this.domSanitizer, serverHost, {}));

			// strip join URL portion
			if (serverHost.startsWith('cfx.re/join/')) {
				serverHost = serverHost.substring(12);
			} else if (serverHost.startsWith('https://cfx.re/join/')) {
				serverHost = serverHost.substring(20);
			}

			// try finding the server in server detail
			if (serverHost.indexOf('.') === -1 && serverHost.indexOf(':') === -1) {
				try {
					server = await this.getServer(serverHost, false);
				} catch {}
			}

			// not found yet? try finding the join ID at least
			if (!server) {
				server = await this.tryGetJoinServer(serverHost);
			}

			// meh, no progress at all. probably private/unlisted
			// try getting dynamic.json to at least populate basic details
			if (!server) {
				try {
				    const serverData = await this.httpClient.post('https://nui-internal/gsclient/dynamic', `url=${serverHost}`, {
                        responseType: 'text'
                    }).pipe(
                        timeout(5000),
                        catchError(e => {
                            console.log('https://nui-internal/gsclient/dynamic timed out');
                            return of(null);
                        })
                    ).toPromise();

					if (serverData) {
						const sd = JSON.parse(serverData);
						sd.addr = serverHost;
						sd.infoBlob = {};

						server = Server.fromNative(this.domSanitizer, sd);
					}
				} catch { }
			}

			if (server) {
				server.connectEndPoints = [serverHost];

				gameService.connectTo(server, serverHost);
			} else {
				const invokeNative: any = (<any>window).invokeNative;

				if (invokeNative) {
					invokeNative('connectTo', serverHost);
				}
			}
		});
	}

	async tryGetJoinServer(serverHost: string) {
		// fetch it
		const serverID = await this.httpClient.post('https://nui-internal/gsclient/url', `url=${serverHost}`, {
			responseType: 'text'
		}).pipe(
			timeout(5000),
			catchError(e => {
				console.log('https://nui-internal/gsclient/url timed out');
				return of(null);
			})
		).toPromise();

		let server: Server = null;

		if (serverID && serverID !== '') {
			try {
				server = await this.getServer(serverID, false);
			} catch {}
		}

		return server;
	}

	deserializeServer(server: master.IServer): master.IServer {
		return server;
	}

	public onInitialized() {
		if (isPlatformBrowser(this.platformId)) {
			this.refreshServers();
		}
	}

	public get serversArray() {
		return Object.values(this.servers);
	}

	private get httpSource() {
		return this.requestEvent
			.asObservable()
			.mergeMap(url => this.httpClient.get(url + 'proto/', { responseType: 'arraybuffer' }))
			.mergeMap(result => master.Servers.decode(new Uint8Array(result)).servers);
	}

	private matchesGame(server: master.IServer) {
		const serverGame = server?.Data?.vars?.gamename || '';

		const localGame = this.gameService.gameName;

		if (serverGame === localGame) {
			return true;
		} else if (serverGame === '' && localGame === 'gta5') {
			return true;
		}

		return false;
	}

	public sortAndFilter(filterRequest: FilterRequest, cb: (servers: string[]) => void) {
		// don't try to sort when we're already trying
		if (this.onSortCB.length > 0 && !filterRequest.fromInteraction) {
			return false;
		}

		this.onSortCB.push(cb);

		this.worker.postMessage({
			type: 'sort',
			data: filterRequest
		});

		return true;
	}

	private refreshServers() {
		this.requestEvent.next('https://servers-frontend.fivem.net/api/servers/');
	}

	public async getServer(address: string, force?: boolean): Promise<Server> {
		if (this.serverCache[address] && this.serverCache[address].isValid() && !force) {
			return this.serverCache[address].server;
		}

		try {
			const server = await this.httpClient.get('https://servers-frontend.fivem.net/api/servers/single/' + address)
				.toPromise()
				.then((data: master.IServer) => Server.fromObject(this.domSanitizer, data.EndPoint, data.Data))
				.catch(() => null);

			if (server) {
				this.serverCache[address] = new ServerCacheEntry(server);
			}

			return server;
		} catch {
			return null;
		}
	}

	getMaterializedServer(server: master.IServer) {
		if (!server) {
			return null;
		}

		if (this.serverCacheMaterialized[server.EndPoint]) {
			return this.serverCacheMaterialized[server.EndPoint];
		}

		const s = Server.fromObject(this.domSanitizer, server.EndPoint, server.Data);
		this.serverCacheMaterialized[server.EndPoint] = s;

		return s;
	}

	public async getTopServer() {
		if (this.topServer !== undefined) {
			return this.topServer;
		}

		const server = await (async () => {
			const languages = this.gameService.systemLanguages;

			for (const language of languages) {
				try {
					return await this.httpClient.get(`https://servers-frontend.fivem.net/api/servers/top/${language}/`)
						.toPromise()
						.then((data: { Data: master.IServer }) => Server.fromObject(this.domSanitizer, data.Data.EndPoint, data.Data.Data));
				} catch {}
			}

			return null;
		})();

		this.topServer = server;

		return server;
	}

	public getServers(): Observable<master.IServer> {
		return this.serversEvent;
	}

	public getCachedServers(): Iterable<master.IServer> {
		return Object.values(this.servers);
	}

	public getReplayedServers(): Observable<master.IServer> {
		return concat(
			from(this.getCachedServers()),
			this.getServers()
		);
	}

	public loadPinConfig(): Promise<PinConfig> {
		return this.httpClient.get('https://runtime.fivem.net/pins.json')
			.toPromise()
			.then((result: PinConfig) => result)
			.catch(() => new PinConfig());
	}

	getNameForPremium(premium: string) {
		switch (premium) {
			case 'pt':
				return 'Element Club Platinum';
			case 'au':
				return 'Element Club Aurum';
			case 'ag':
				return 'Element Club Argentum';
		}

		return '';
	}

	parseAddress(addr: string): [string, number] {
		if (!addr) {
			return null;
		}

		const addrBits: [string, number] = ['', 30120];
		const match = addr.match(/^(?:((?:[^\[: ]+)|\[(?:[a-f0-9:]+)\])(?::([0-9]+)|$)|cfx\.re\/join\/[0-9a-z]+)/i);

		if (!match) {
			return [addr, 0];
		}

		addrBits[0] = match[1];

		if (match[2]) {
			addrBits[1] = parseInt(match[2], 10);
		}

		return addrBits;
	}
}
