import { Injectable } from '@angular/core';
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

import idb from 'idb';

// this class loosely based on https://github.com/rkusa/frame-stream
class FrameReader {
    private reader: ReadableStreamReader;

    public frame = new Subject<Uint8Array>();
    public end = new Subject<void>();

    private lastArray: Uint8Array;
    private frameLength = -1;
    private framePos = 0;

    constructor(stream: ReadableStream) {
        this.reader = stream.getReader();
    }

    public beginRead() {
        this.reader.read().then(({done, value}) => {
            if (done) {
                this.end.next();
                return;
            }

            let array: Uint8Array = value;

            while (array.length > 0) {
                const start = 4;

                if (this.lastArray) {
                    const newArray = new Uint8Array(array.length + this.lastArray.length);
                    newArray.set(this.lastArray);
                    newArray.set(array, this.lastArray.length);

                    this.lastArray = null;

                    array = newArray;
                }

                if (this.frameLength < 0) {
                    if (array.length < 4) {
                        this.lastArray = array;
                        this.beginRead();
                        return;
                    }

                    this.frameLength = array[0] | (array[1] << 8) | (array[2] << 16) | (array[3] << 24);

                    if (this.frameLength > 65535) {
                        throw new Error('A too large frame was passed.');
                    }
                }

                const end = 4 + this.frameLength - this.framePos;

                if (array.length < end) {
                    this.lastArray = array;
                    this.beginRead();
                    return;
                }

                const bit = array.slice(start, end);
                this.framePos += (end - start);

                if (this.framePos === this.frameLength) {
                    // reset
                    this.frameLength = -1;
                    this.framePos = 0;
                }

                this.frame.next(bit);

                // more in the array?
                if (array.length > end) {
                    array = array.slice(end);
                } else {
                    // continue reading
                    this.beginRead();

                    return;
                }
            }
        });
    }
}

@Injectable()
export class ServersService {
    private requestEvent: Subject<string>;
    private serversEvent: Subject<Server>;

    constructor(private http: Http, private httpClient: HttpClient, private domSanitizer: DomSanitizer) {
        this.requestEvent = new Subject<string>();

        this.serversEvent = new Subject<Server>();

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
        return this.requestEvent
            .asObservable()
            .mergeMap(url => Observable.fromPromise(fetch(new Request(url))))
            .mergeMap(response => {
                const subject = new Subject<master.IServer>();
                const frameReader = new FrameReader(response.body);

                frameReader.frame
                    .subscribe(message => subject.next(master.Server.decode(message)))

                frameReader.end.subscribe(() => subject.complete());

                frameReader.beginRead();

                return subject;
            });
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
