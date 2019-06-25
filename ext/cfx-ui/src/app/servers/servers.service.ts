import { Injectable, NgZone, Inject, PLATFORM_ID } from '@angular/core';
import { Http, ResponseContentType } from '@angular/http';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer } from '@angular/platform-browser';

import { Observable } from 'rxjs/Observable';
import { Subject } from 'rxjs/Subject';

import { applyPatch } from 'fast-json-patch';

import { concat, from } from 'rxjs';

import 'rxjs/add/observable/fromPromise';
import 'rxjs/add/operator/bufferTime';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/mergeMap';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

import ReconnectingWebSocket from 'reconnecting-websocket';

import { Server, ServerIcon, PinConfig } from './server';

import { master } from './master';
import { isPlatformBrowser } from '@angular/common';

const serverWorker = require('file-loader?name=worker.[hash:20].[ext]!../../worker/index.js');

@Injectable()
export class ServersService {
    private requestEvent: Subject<string>;
    private serversEvent: Subject<Server>;

    private internalServerEvent: Subject<master.IServer>;

    private worker: Worker;

    private webSocket: ReconnectingWebSocket;

    private servers: {[ addr: string ]: Server} = {};

    constructor(private httpClient: HttpClient, private domSanitizer: DomSanitizer, private zone: NgZone,
        @Inject(PLATFORM_ID) private platformId: any) {
        this.requestEvent = new Subject<string>();

        this.serversEvent = new Subject<Server>();
        this.internalServerEvent = new Subject<master.IServer>();

        // only enable the worker if streams are supported
        if (typeof window !== 'undefined' && window.hasOwnProperty('Response') && Response.prototype.hasOwnProperty('body')) {
            this.worker = new Worker(serverWorker);
            zone.runOutsideAngular(() => {
                this.worker.addEventListener('message', (event) => {
                    if (event.data.type === 'addServers') {
                        for (const server of event.data.servers) {
                            this.internalServerEvent.next(server);
                        }
                    } else if (event.data.type === 'serversDone') {
                        this.internalServerEvent.next(null);
                    }
                });
            });

            this.requestEvent
                .subscribe(url => {
                    this.worker.postMessage({ type: 'queryServers', url: url + 'stream/' });
                });

            this.subscribeWebSocket();
        }

        this.serversSource
            .filter(a => !a || a.Data != null)
            .map(value => value ? Server.fromObject(this.domSanitizer, value.EndPoint, value.Data) : null)
            .subscribe(server => {
                if (!server) {
                    this.serversEvent.next(null);
                    return;
                }

                this.servers[server.address] = server;
                this.serversEvent.next(server);
            });

        if (isPlatformBrowser(this.platformId)) {
            this.refreshServers();
        }
    }

    private get serversSource(): Observable<master.IServer> {
        if (typeof window !== 'undefined' && window.hasOwnProperty('Response') && Response.prototype.hasOwnProperty('body')) {
            return this.fetchSource;
        } else {
            return this.httpSource;
        }
    }

    private get fetchSource() {
        return this.internalServerEvent;
    }

    private get httpSource() {
        return this.requestEvent
            .asObservable()
            .mergeMap(url => this.httpClient.get(url + 'proto/', { responseType: 'arraybuffer' }))
            .mergeMap(result => master.Servers.decode(new Uint8Array(result)).servers);
    }

    private subscribeWebSocket() {
        const ws = new ReconnectingWebSocket('wss://servers-live.fivem.net/api/servers/socket/v1/');
        ws.addEventListener('message', (ev) => {
            const data = JSON.parse(ev.data);

            switch (data.op) {
                case 'ADD_SERVER':
                    this.internalServerEvent.next({
                        Data: data.data.data,
                        EndPoint: data.id
                    });
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
        this.requestEvent.next('https://servers-live.fivem.net/api/servers/');
    }

    public getServer(address: string): Promise<Server> {
        return this.httpClient.get('https://servers-live.fivem.net/api/servers/single/' + address)
            .toPromise()
            .then((data: master.IServer) => Server.fromObject(this.domSanitizer, data.EndPoint, data.Data));
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
}
