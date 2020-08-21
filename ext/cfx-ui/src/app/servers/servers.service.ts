import { Injectable, NgZone, Inject, PLATFORM_ID } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer, SafeUrl } from '@angular/platform-browser';

import { Observable } from 'rxjs/Observable';
import { Subject } from 'rxjs/Subject';

import { applyPatch } from 'fast-json-patch';

import { concat, from, BehaviorSubject } from 'rxjs';

import 'rxjs/add/observable/fromPromise';
import 'rxjs/add/operator/bufferTime';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/mergeMap';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

import ReconnectingWebSocket from 'reconnecting-websocket';

import { Server, ServerHistoryEntry } from './server';
import { PinConfig } from './pins';

import { master } from './master';
import { isPlatformBrowser } from '@angular/common';
import { GameService } from '../game.service';
import { FilterRequest } from './filter-request';

const serversWorker = new Worker('./workers/servers.worker', { type: 'module' });

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

@Injectable()
export class ServersService {
	private requestEvent: Subject<string>;
	private serversEvent: Subject<Server>;

	private internalServerEvent: Subject<master.IServer>;

	private worker: Worker;

	private webSocket: ReconnectingWebSocket;

	public servers: { [addr: string]: Server } = {};
	public serversLoadedUpdate: Subject<boolean> = new BehaviorSubject(false);

	private serverCache: { [addr: string]: ServerCacheEntry } = {};

	private onSortCB: ((servers: string[]) => void)[] = [];

	constructor(private httpClient: HttpClient, private domSanitizer: DomSanitizer, private zone: NgZone,
		private gameService: GameService, @Inject(PLATFORM_ID) private platformId: any) {
		this.requestEvent = new Subject<string>();

		this.serversEvent = new Subject<Server>();
		this.internalServerEvent = new Subject<master.IServer>();

		// only enable the worker if streams are supported
		if (typeof window !== 'undefined' && window.hasOwnProperty('Response') && Response.prototype.hasOwnProperty('body')) {
			this.worker = serversWorker;
			zone.runOutsideAngular(() => {
				this.worker.addEventListener('message', (event) => {
					if (event.data.type === 'addServers') {
						for (const server of event.data.servers) {
							if (this.matchesGame(server)) {
								this.internalServerEvent.next(server);
							}
						}
					} else if (event.data.type === 'serversDone') {
						this.internalServerEvent.next(null);
						this.serversLoadedUpdate.next(true);
					} else if (event.data.type === 'sortedServers') {
						if (this.onSortCB.length) {
							zone.run(() => {
								this.onSortCB[0](event.data.servers);
								this.onSortCB.shift();
							});
						}
					} else if (event.data.type === 'pushBitmap') {
						const addr: string = event.data.server;
						const bitmap: ImageBitmap = event.data.bitmap;

						this.servers[addr].bitmap = bitmap;
					} else {
						console.log('[servers] worker message rcv', event);
					}
				});
			});

			this.requestEvent
				.subscribe(url => {
					this.worker.postMessage({ type: 'queryServers', url: url + `streamRedir/` });
				});

			this.subscribeWebSocket();
		}

		this.serversSource
			.filter(a => !a || a.Data != null)
			.map((server) => {
				if (server?.Data) {
					return Server.fromObject(this.domSanitizer, server.EndPoint, server.Data)
				}

				return null;
			})
			.subscribe(server => {
				if (!server) {
					this.serversEvent.next(null);
					return;
				}

				this.servers[server.address] = server;
				this.serversEvent.next(server);
			});
	}

	public onInitialized() {
		if (isPlatformBrowser(this.platformId)) {
			this.refreshServers();
		}
	}

	public get serversArray() {
		return Object.values(this.servers);
	}

	private get serversSource(): Observable<master.IServer> {
		if (typeof window !== 'undefined' && window.hasOwnProperty('Response') && Response.prototype.hasOwnProperty('body')) {
			return this.internalServerEvent;
		} else {
			return this.httpSource;
		}
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

	private subscribeWebSocket() {
		const ws = new ReconnectingWebSocket('wss://servers-frontend.fivem.net/api/servers/socket/v1/');
		ws.addEventListener('message', (ev) => {
			const data = JSON.parse(ev.data);

			switch (data.op) {
				case 'ADD_SERVER':
					const server = {
						Data: data.data.data,
						EndPoint: data.id
					};

					if (this.matchesGame(server)) {
						this.internalServerEvent.next(server);
					}
					break;
				case 'UPDATE_SERVER':
					const old = this.servers[data.id];

					if (old) {
						const patch = data.data;
						const result = applyPatch({ data: old.data }, patch).newDocument;

						const ping = old.ping;
						result.data.vars.ping = ping;

						this.internalServerEvent.next({
							Data: result.data,
							EndPoint: data.id
						});
					}
					break;
				case 'REMOVE_SERVER':
					// not impl'd
					break;
			}
		});

		this.webSocket = ws;
	}

	private refreshServers() {
		this.requestEvent.next('https://servers-frontend.fivem.net/api/servers/');
	}

	public async getServer(address: string, force?: boolean): Promise<Server> {
		if (this.serverCache[address] && this.serverCache[address].isValid() && !force) {
			return this.serverCache[address].server;
		}

		const server = await this.httpClient.get('https://servers-frontend.fivem.net/api/servers/single/' + address)
			.toPromise()
			.then((data: master.IServer) => Server.fromObject(this.domSanitizer, data.EndPoint, data.Data));

		this.serverCache[address] = new ServerCacheEntry(server);

		return server;
	}

	public getServers(): Observable<Server> {
		return this.serversEvent;
	}

	public getCachedServers(): Iterable<Server> {
		return Object.values(this.servers);
	}

	public getReplayedServers(): Observable<Server> {
		return concat(
			from(this.getCachedServers()),
			this.getServers()
		);
	}

	public loadPinConfig(): Promise<PinConfig> {
		return this.httpClient.get('https://runtime.fivem.net/pins.json')
			.toPromise()
			.then((result: PinConfig) => result);
	}

	parseAddress(addr: string): [string, number] {
		if (!addr) {
			return null;
		}

		const addrBits: [string, number] = ['', 30120];
		const match = addr.match(/^(?:((?:[^\[: ]+)|\[(?:[a-f0-9:]+)\])(?::([0-9]+)|$)|cfx\.re\/join\/[0-9a-z]+)/i);

		if (!match) {
			return null;
		}

		addrBits[0] = match[1];

		if (match[2]) {
			addrBits[1] = parseInt(match[2], 10);
		}

		return addrBits;
	}
}
