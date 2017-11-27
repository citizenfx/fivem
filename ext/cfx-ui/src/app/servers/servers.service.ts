import { Injectable, NgZone } from '@angular/core';
import { Http, ResponseContentType } from '@angular/http';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer } from '@angular/platform-browser';

import { Observable } from 'rxjs/Observable';
import { Subject } from 'rxjs/Subject';

import 'rxjs/add/observable/fromPromise';
import 'rxjs/add/operator/bufferTime';
import 'rxjs/add/operator/toPromise';
import 'rxjs/add/operator/mergeMap';
import 'rxjs/add/operator/filter';
import 'rxjs/add/operator/map';

import { Server, ServerIcon, PinConfig } from './server';

import { master } from './master';

const serverWorker = require('file-loader?name=worker.[hash:20].[ext]!../../worker/index.js');

@Injectable()
export class ServersService {
    private requestEvent: Subject<string>;
    private serversEvent: Subject<Server>;

    private internalServerEvent: Subject<master.IServer>;

    private worker: Worker;

    constructor(private http: Http, private httpClient: HttpClient, private domSanitizer: DomSanitizer, private zone: NgZone) {
        this.requestEvent = new Subject<string>();

        this.serversEvent = new Subject<Server>();
        this.internalServerEvent = new Subject<master.IServer>();

        this.worker = new Worker(serverWorker);
        zone.runOutsideAngular(() => {
            this.worker.addEventListener('message', (event) => {
                if (event.data.type === 'addServers') {
                    for (const server of event.data.servers) {
                        this.internalServerEvent.next(server);
                    }
                }
            });
        });
        this.requestEvent.subscribe(url => {
            this.worker.postMessage({ type: 'queryServers', url });
        })

        this.serversSource
            .filter(a => a.Data != null)
            .map(value => Server.fromObject(this.domSanitizer, value.EndPoint, value.Data))
            .subscribe(server => this.serversEvent.next(server));
    }

    private get serversSource(): Observable<master.IServer> {
        if (Response !== undefined && Response.prototype.hasOwnProperty('body')) {
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
            .mergeMap(url => this.httpClient.get(url, { responseType: 'arraybuffer' }))
            .mergeMap(result => master.Servers.decode(new Uint8Array(result)).servers);
    }

    public refreshServers() {
        this.requestEvent.next('https://runtime.fivem.net/api/servers/stream/');
    }

    public getServers(): Observable<Server> {
        return this.serversEvent;
    }

    public loadPinConfig(): Promise<PinConfig> {
        return this.http.get('https://runtime.fivem.net/pins.json')
            .toPromise()
            .then(result => <PinConfig>result.json());
    }
}
